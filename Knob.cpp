#include "Knob.h"

#include <Energia.h>
#include <cstdarg>

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

Handler::Handler( callback_t callback ) : _cb( callback ) {}


// == Push ==

Push::Push( callback_t callback ) : Handler( callback ) {}

bool Push::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState && !oldState )
			return _cb( dev, newState, oldState, time );

	return true;
}


// == Release ==

Release::Release( callback_t callback ) : Handler( callback ) {}

bool Release::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( !newState && oldState )
			return _cb( dev, newState, oldState, time );

	return true;
}


// == Click ==

Click::Click( callback_t callback, time_t maxTime )
		: Handler( callback )
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
					return _cb( dev, newState, oldState, time );

		}

	}

	return true;

}

// == Hold ==

Hold::Hold( callback_t callback, time_t time ) 
		: Handler( callback ) 
		, _timeHold( time )
		{
		
	_continues = false;
	_hasSent = false;
}

bool Hold::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState && time >= _timeHold && ( !_hasSent || _continues ) ) {

		_hasSent = true;

		return _cb( dev, newState, oldState, time );
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
