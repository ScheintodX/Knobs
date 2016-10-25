#include "Knob.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

// == Device ==

Device& Device::on( Handler &handler ){

	handler._next = _firstHandler;
	_firstHandler = &handler;

	return *this;
}

void Device::_activate( bool newState, bool oldState, time_t time ) {

	Handler *handler;
	bool cont;

	for( handler = _firstHandler; handler; handler = handler->_next ) {

		cont = handler->handle( *this, newState, oldState, time );

		if( !cont ) break;
	}
}


// == BooleanDevice ==

BooleanDevice::BooleanDevice( pin_t pin) : _pin( pin ) {

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


// == Knob ==

Knob::Knob( uint8_t pin ) 
		: BooleanDevice( pin )
{

	_value = 0;

	_lastTime = 0;

	_timeDebounce = TIME_DEBOUNCE;
	_countDebounce = 0;

	pinMode( pin, INPUT );
}

value_t Knob::value() {

	return _value;
}

static inline time_t MIN( time_t v1, time_t v2 ) {

	return v1 < v2 ? v1 : v2;
}
static inline time_t MAX( time_t v1, time_t v2 ) {

	return v1 > v2 ? v1 : v2;
}

void Knob::loop() {

	time_t now = millis(),
	       delta = _lastTime < now ?
		   		now - _lastTime :
				(((time_t)-1) - _lastTime) + now +1; // compensate for overrun every 50 days
	       ;

	value_t oldValue = _value,
	        value = _read()
	        ;


	_lastTime = now;

	//Serial.print( value );

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

Knob& Knob::debounce( time_t time ) {
	_timeDebounce = time;
	return *this;
}


/*
 * H A N D L E R
 */

Handler::Handler( HandlerType type, callback_t callback ) 
		: _cb( callback ), _cbm( NULL ), type( type ) {}
Handler::Handler( HandlerType type, minimal_callback_t callback )
		: _cb( NULL ), _cbm( callback ), type( type ) {}

bool Handler::_callback( Device &dev, value_t newState, value_t oldState, time_t count ) {

	if( _cbm ) {
		_cbm();
		return true;
	//} else if( _cbs ) return _cbs( newState, oldState, count );
	} else return _cb( dev, *this, newState, oldState, count );
}


// == Push ==

Push::Push( callback_t callback ) : Handler( PUSH, callback ) {}
Push::Push( minimal_callback_t callback ) : Handler( PUSH, callback ) {}

bool Push::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState && !oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}


// == Release ==

Release::Release( callback_t callback ) : Handler( RELEASE, callback ) {}
Release::Release( minimal_callback_t callback ) : Handler( RELEASE, callback ) {}

bool Release::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( !newState && oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}


// == Toggle ==

Toggle::Toggle( callback_t callback ) : Handler( TOGGLE, callback ) {}
Toggle::Toggle( minimal_callback_t callback ) : Handler( TOGGLE, callback ) {}

bool Toggle::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState != oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}


// == Click ==

Click::Click( HandlerType type, callback_t callback, time_t maxTime )
		: Handler( type, callback )
		, _maxTimeClick( maxTime )
		{}
Click::Click( HandlerType type, minimal_callback_t callback, time_t maxTime )
		: Handler( type, callback )
		, _maxTimeClick( maxTime )
		{}

Click::Click( callback_t callback )
		: Handler( CLICK, callback )
		, _maxTimeClick( MAX_TIME_CLICK )
		{}

Click::Click( callback_t callback, time_t maxTime )
		: Handler( CLICK, callback )
		, _maxTimeClick( maxTime )
		{}

Click::Click( minimal_callback_t callback )
		: Handler( CLICK, callback )
		, _maxTimeClick( MAX_TIME_CLICK )
		{}

Click::Click( minimal_callback_t callback, time_t maxTime )
		: Handler( CLICK, callback )
		, _maxTimeClick( maxTime )
		{}

bool Click::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	time_t now = millis();

	if( newState != oldState ) {

		// press
		if( newState ) {

			_timeStart = now;

		// release
		} else {

			if( now-_timeStart < _maxTimeClick )
					return _callback( dev, newState, oldState, time );

		}
	}

	return true;
}

// == DoubleClick ==

DoubleClick::DoubleClick( callback_t callback )
		: Click( DOUBLECLICK, callback, MAX_TIME_CLICK )
		, _maxTimeInbetween( MAX_TIME_INBETWEEN )
		, _maxClicks( 2 )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}

DoubleClick::DoubleClick( callback_t callback, value_t maxClicks )
		: Click( DOUBLECLICK, callback, MAX_TIME_CLICK )
		, _maxTimeInbetween( MAX_TIME_INBETWEEN )
		, _maxClicks( 2 )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}

DoubleClick::DoubleClick( callback_t callback, value_t maxClicks, time_t maxTimeClick, time_t maxTimeInbetween )
		: Click( DOUBLECLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		, _maxClicks( maxClicks )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}

DoubleClick::DoubleClick( minimal_callback_t callback )
		: Click( DOUBLECLICK, callback, MAX_TIME_CLICK )
		, _maxTimeInbetween( MAX_TIME_INBETWEEN )
		, _maxClicks( 2 )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}

DoubleClick::DoubleClick( minimal_callback_t callback, value_t maxClicks )
		: Click( DOUBLECLICK, callback, MAX_TIME_CLICK )
		, _maxTimeInbetween( MAX_TIME_INBETWEEN )
		, _maxClicks( 2 )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}

DoubleClick::DoubleClick( minimal_callback_t callback, value_t maxClicks, time_t maxTimeClick, time_t maxTimeInbetween )
		: Click( DOUBLECLICK, callback, maxTimeClick )
		, _maxTimeInbetween( maxTimeInbetween )
		, _maxClicks( maxClicks )
		{
	
	_clicks = 0;
	_timeStartSequence = 0;
}


bool DoubleClick::_callback( Device &dev, value_t newValue, value_t oldValue, time_t count ) {

	time_t now = millis(),
	       delta = now-_timeStartSequence
		   ;

	if( _clicks == 0 ) {

		_timeStartSequence = now;
		_clicks = 1;

	} else {

		if( delta < _maxTimeInbetween ) {
			_clicks ++;
		} else {
			_clicks = 0;
		}

		if( _clicks == _maxClicks ) {

			_clicks = 0;

			return Click::_callback( dev, newValue, oldValue, delta );
		}
	}

	return true;
}

bool DoubleClick::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	return Click::handle( dev, newState, oldState, time );

}

// == Hold ==

Hold::Hold( callback_t callback, time_t time ) 
		: Handler( HOLD, callback ) 
		, _timeHold( time )
		{
		
	_continues = false;
	_hasSent = false;
}
Hold::Hold( minimal_callback_t callback, time_t time ) 
		: Handler( HOLD, callback ) 
		, _timeHold( time )
		{
		
	_continues = false;
	_hasSent = false;
}

bool Hold::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState && time >= _timeHold && ( !_hasSent || _continues ) ) {

		_hasSent = true;

		return _callback( dev, newState, oldState, time );
	} 
	if( !newState ) {

		_hasSent = false;
	}

	return true;
}

Hold& Hold::continues( bool on ) {

	_continues = on;

	return *this;
}



/*
 * ## P A N E L ##
 */

Panel::Panel(){}

Panel& Panel::operator <<( Device &dev ) {

	dev._next = _first;
	_first = &dev;

	return *this;
}

void Panel::loop() {

	//Serial.print( "." );

	Device *dev;

	for( dev = _first; dev; dev = dev->_next ) {

		dev->loop();
	}
}

#pragma GCC diagnostic pop
