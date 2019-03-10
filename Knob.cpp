#include "Knob.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

static inline knob_time_t MIN( knob_time_t v1, knob_time_t v2 ) {
	return v1 < v2 ? v1 : v2;
}
static inline knob_time_t MAX( knob_time_t v1, knob_time_t v2 ) {
	return v1 > v2 ? v1 : v2;
}

/*****************************************************************************
*
*   D e v i c e
*
*****************************************************************************/

Device::Device( const char *name ) : _name( name ) {

	_mute = false;
	_slave = NULL;
}

Device& Device::enslave( Device &slave ){
	slave.mute( true );
	_slave = &slave;
	return *this;
}
Device& Device::mute( bool mute ) {
	_mute = mute;
	return *this;
}
bool Device::mute() {
	return _mute;
}

Device& Device::on( Handler &handler ){

	_handlers.add( handler );

	return *this;
}

const char* Device::name() {
	return _name;
}

void Device::_activate( knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	Handler *handler;

	bool cont;

	for( handler = _handlers.first(); handler; handler = _handlers.next() ) {

		cont = handler->handle( *this, newState, oldState, time );

		if( !cont ) break;
	}

	if( _slave ) _slave->_activate( newState, oldState, time );
}


/*****************************************************************************
*
*   B O O L E A N  D E V I C E
*
*****************************************************************************/

BooleanDevice::BooleanDevice( const char *name, pin_t pin ) : Device( name ), _pin( pin ) {

	_invert = false;
}

BooleanDevice& BooleanDevice::pullup( bool on ) {

	pinMode( _pin, on ? INPUT_PULLUP : INPUT );

	return *this;
}
BooleanDevice& BooleanDevice::invert( bool on ) {

	_invert = on;

	return *this;
}
bool BooleanDevice::_read() {

	bool val = digitalRead( _pin );

	return _invert ? !val : val;
}

pin_t BooleanDevice::pin() {

	return _pin;
}


/*****************************************************************************
*
*   K N O B
*
*****************************************************************************/

Knob::Knob( const char *name, pin_t pin )
		: BooleanDevice( name, pin )
{

	_value = 0;

	_lastTime = 0;
	_timeUnchanged = 0;

	_timeDebounce = TIME_DEBOUNCE;
	_countDebounce = 0;

	pinMode( pin, INPUT );
}

knob_value_t Knob::value() {

	return _value;
}

void Knob::loop() {

	if( mute() ) return;

	knob_time_t now = millis(),
	       delta = _lastTime < now ?
		   		now - _lastTime :
				(((knob_time_t)-1) - _lastTime) + now +1; // compensate for overrun every 50 days
	       ;

	knob_value_t oldValue = _value,
	        value = _read()
	        ;

	_lastTime = now;

	if( value ) {

		_countDebounce = MIN( _timeDebounce, _countDebounce + delta );

	} else {
		_countDebounce = _countDebounce > delta ? _countDebounce-delta : 0;
	}

	if( _countDebounce == 0 || _countDebounce == _timeDebounce ) {

		if( value != _value )
				_timeUnchanged = 0;

		_timeUnchanged += delta;

		_value = value;

		_activate( value, oldValue, _timeUnchanged );
	}
}

Knob& Knob::debounce( knob_time_t time ) {
	_timeDebounce = time;
	return *this;
}


/*****************************************************************************
*
*   H A N D L E R
*
*****************************************************************************/

Handler::Handler( HandlerType type, callback_t callback )
		: _cb( callback ), _cbm( NULL ), _ca( NULL ), type( type ) {}
Handler::Handler( HandlerType type, minimal_callback_t callback )
		: _cb( NULL ), _cbm( callback ), _ca( NULL ), type( type ) {}
Handler::Handler( HandlerType type, Callable &callable )
		: _cb( NULL ), _cbm( NULL ), _ca( &callable ), type( type ) {}

bool Handler::_callback( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t count ) {

	if( _cbm ) {
		return _cbm( newState );
	} else if( _cb ) {
		return _cb( dev, *this, newState, oldState, count );
	} else {
		return _ca->call( dev, *this, newState, oldState, count );
	}
		//virtual bool call( Device &dev, Handler &handler,
				//knob_value_t newState, knob_value_t oldState, knob_time_t count ) = 0;
}


// == Always ==

Always::Always( callback_t callback ) : Handler( HT_ALWAYS, callback ) {}
Always::Always( minimal_callback_t callback ) : Handler( HT_ALWAYS, callback ) {}

bool Always::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	return _callback( dev, newState, oldState, time );
}

// == Push ==

Push::Push( callback_t callback ) : Handler( HT_PUSH, callback ) {}
Push::Push( minimal_callback_t callback ) : Handler( HT_PUSH, callback ) {}

bool Push::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( newState && !oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}


// == Release ==

Release::Release( callback_t callback ) : Handler( HT_RELEASE, callback ) {}
Release::Release( minimal_callback_t callback ) : Handler( HT_RELEASE, callback ) {}

bool Release::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( !newState && oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}


// == Toggle ==

Toggle::Toggle( callback_t callback ) : Handler( HT_TOGGLE, callback ) {}
Toggle::Toggle( minimal_callback_t callback ) : Handler( HT_TOGGLE, callback ) {}

bool Toggle::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( newState != oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}

// == Transport ==
// (callback regularily if active, deactove one time)

Transport::Transport( callback_t callback, knob_time_t periode )
		: Handler( HT_TRANSPORT, callback ), _periode( periode ){
		
	_lastTime = 0;
}
Transport::Transport( minimal_callback_t callback, knob_time_t periode )
		: Handler( HT_TRANSPORT, callback ), _periode( periode ){
	
	_lastTime = 0;
}

bool Transport::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( !oldState && newState ) {
		_lastTime = time;
		return _callback( dev, newState, oldState, time );
	}

	if( newState && time - _lastTime > _periode ) {
		_lastTime = time;
		return _callback( dev, newState, oldState, time );
	}

	if( oldState && !newState ) {
		return _callback( dev, newState, oldState, time );
	}

	return true;
}


// == Click ==

Click::Click( HandlerType type, callback_t callback, knob_time_t maxTimeClick )
		: Handler( type, callback )
		, _maxTimeClick( maxTimeClick ) {

	_timeStart = 0;
}
Click::Click( HandlerType type, minimal_callback_t callback, knob_time_t maxTimeClick )
		: Handler( type, callback )
		, _maxTimeClick( maxTimeClick ) {

	_timeStart = 0;
}

Click::Click( callback_t callback, knob_time_t maxTimeClick )
		: Handler( HT_CLICK, callback )
		, _maxTimeClick( maxTimeClick ) {

	_timeStart = 0;
}

Click::Click( minimal_callback_t callback, knob_time_t maxTimeClick )
		: Handler( HT_CLICK, callback )
		, _maxTimeClick( maxTimeClick ) {

	_timeStart = 0;
}

bool Click::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	knob_time_t now = millis();

	if( newState != oldState ) {

		// press
		if( newState ) {

			_timeStart = now;

		// release
		} else {

			if( !_maxTimeClick || now-_timeStart < _maxTimeClick )
					return _callback( dev, newState, oldState, time );

		}
	}

	return true;
}

// == DoubleClick ==

DoubleClick::DoubleClick( callback_t callback,
		knob_value_t maxClicks, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween )
		: Click( HT_DOUBLECLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		, _maxClicks( maxClicks ) {

	_timeLastClick = 0;
	_clicks = 0;
}

DoubleClick::DoubleClick( minimal_callback_t callback,
		knob_value_t maxClicks, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween )
		: Click( HT_DOUBLECLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		, _maxClicks( maxClicks ) {

	_timeLastClick = 0;
	_clicks = 0;
}

bool DoubleClick::_callback( Device &dev, knob_value_t newValue, knob_value_t oldValue, knob_time_t count ) {

	knob_time_t now = millis(),
	       delta = now-_timeLastClick
		   ;
	
	_timeLastClick = now;

	if( delta > _maxTimeInbetween ) {
		_clicks = 0;
	}

	if( _clicks == 0 ) {

		_clicks = 1;

	} else {

		_clicks ++;

		if( _clicks == _maxClicks ) {

			_clicks = 0;

			return Click::_callback( dev, newValue, oldValue, delta );
		}
	}

	return true;
}

bool DoubleClick::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	return Click::handle( dev, newState, oldState, time );

}


// == MultiClick ==

MultiClick::MultiClick( callback_t callback, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween )
		: Click( HT_MULTICLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		{

	_timeLastClick = 0;
	_clicks = 0;
}

MultiClick::MultiClick( minimal_callback_t callback, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween )
		: Click( HT_MULTICLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		{

	_timeLastClick = 0;
	_clicks = 0;
}


bool MultiClick::_callback( Device &dev, knob_value_t newValue, knob_value_t oldValue, knob_time_t count ) {

	knob_time_t now = millis(),
	       delta = now-_timeLastClick
		   ;

	_timeLastClick = now;

	if( delta < _maxTimeInbetween ) {
		_clicks ++;
		return Click::_callback( dev, _clicks, _clicks-1, delta );
	} else {
		_clicks = 0;
	}

	return true;
}

bool MultiClick::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	return Click::handle( dev, newState, oldState, time );

}

// == Hold ==

Hold::Hold( callback_t callback, knob_time_t time )
		: Handler( HT_HOLD, callback )
		, _timeHold( time ) {

	_continues = false;
	_hasSent = false;
}

Hold::Hold( minimal_callback_t callback, knob_time_t time )
		: Handler( HT_HOLD, callback )
		, _timeHold( time ) {

	_continues = false;
	_hasSent = false;
}

bool Hold::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( newState && time >= _timeHold && ( !_hasSent || _continues ) ) {

		_hasSent = true;

		return _callback( dev, newState, oldState, time );
	}
	if( !newState ) {

		if( _hasSent ) {
			_hasSent = false;
			return false; // keep click from been called (if click is *after* this)
		}
	}

	return true;
}

Hold& Hold::continues( bool on ) {

	_continues = on;

	return *this;
}



// === Over ====

Over::Over( callback_t callback, knob_value_t val )
		: Handler( HT_OVER, callback )
		, _val( val )
		{ }

Over::Over( minimal_callback_t callback, knob_value_t val )
		: Handler( HT_OVER, callback )
		, _val( val )
		{ }

bool Over::handle( Device &dev,
		knob_value_t newState, knob_value_t oldState, knob_time_t time ){

	if( oldState < _val && newState >= _val )
			return _callback( dev, newState, oldState, time );

	return false;

}


// === Under ====

Under::Under( callback_t callback, knob_value_t val )
		: Handler( HT_UNDER, callback )
		, _val( val )
		{ }

Under::Under( minimal_callback_t callback, knob_value_t val )
		: Handler( HT_UNDER, callback )
		, _val( val )
		{ }

bool Under::handle( Device &dev,
		knob_value_t newState, knob_value_t oldState, knob_time_t time ){

	if( oldState > _val && newState <= _val )
			return _callback( dev, newState, oldState, time );

	return false;

}

// === Hyseresis ====

Hysteresis::Hysteresis( callback_t callback,
		knob_value_t lower_bound, knob_value_t upper_bound )
		: Handler( HT_OVER, callback )
		, _upper_bound( upper_bound )
		, _lower_bound( lower_bound )
		{ }

Hysteresis::Hysteresis( minimal_callback_t callback,
		knob_value_t lower_bound, knob_value_t upper_bound )
		: Handler( HT_OVER, callback )
		, _upper_bound( upper_bound )
		, _lower_bound( lower_bound )
		{ }

bool Hysteresis::handle( Device &dev,
		knob_value_t newState, knob_value_t oldState, knob_time_t time ){

	if( oldState < _upper_bound && newState >= _upper_bound )
			return _callback( dev, newState, oldState, time );
	if( oldState > _lower_bound && newState <= _lower_bound )
			return _callback( dev, newState, oldState, time );

	return false;
}



// === SlowAveragingHysteresis ===

SlowAveragingHysteresis::SlowAveragingHysteresis( callback_t callback,
		float lower_bound, float upper_bound, int averaging )
		: Handler( HT_OVER, callback )
		, _upper_bound( upper_bound )
		, _lower_bound( lower_bound )
		, _averaging( averaging )
		{

	_lastTime = millis();
	_value = 0;
}

bool SlowAveragingHysteresis::handle( Device &dev,
		knob_value_t newState, knob_value_t oldState, knob_time_t time ){

	time_t now = millis();
	if( now < (_lastTime + _DELAY) ) return false;
	_lastTime = time;

	float oldValue = _value,
	      newValue = _value = ( _value * (_averaging-1) + newState ) / _averaging;

	_value = newValue;

	// Note that we dont supply real value but simply 0/1
	if( oldValue < _upper_bound && newValue >= _upper_bound )
			return _callback( dev, 1, 0, time );
	if( oldValue > _lower_bound && newValue <= _lower_bound )
			return _callback( dev, 0, 1, time );

	return false;
}
/*
 * ## P A N E L ##
 */

Panel::Panel( const char *name )
		: _name( name ){}

Panel::Panel( const char *name, Device &k1 )
		: _name( name ){
	*this << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2 )
		: _name( name ){
	*this << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3 )
		: _name( name ){
	*this << k3 << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4 )
		: _name( name ){
	*this << k4 << k3 << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
		Device &k5 )
		: _name( name ){
	*this << k5 << k4 << k3 << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
		Device &k5, Device &k6 )
		: _name( name ){
	*this << k6 << k5 << k4 << k3 << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
		Device &k5, Device &k6, Device &k7 )
		: _name( name ){
	*this << k7 << k6 << k5 << k4 << k3 << k2 << k1;
}
Panel::Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
		Device &k5, Device &k6, Device &k7, Device &k8 )
		: _name( name ) {
	*this << k8 << k7 << k6 << k5 << k4 << k3 << k2 << k1;
}

Panel& Panel::operator <<( Device &dev ) {

	_devices.add( dev );

	return *this;
}

void Panel::loop() {

	Device *dev;

	for( dev = _devices.first(); dev; dev = _devices.next() ) {

		dev->loop();
	}
}

const char * Panel::name() {
	return _name;
}

#pragma GCC diagnostic pop
