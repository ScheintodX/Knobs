#include "Lever.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

/*****************************************************************************
*
*   A n a l o g  D e v i c e
*
*****************************************************************************/

AnalogDevice::AnalogDevice( pin_t pin ) 
		: _pin( pin ) {

	pinMode( pin, INPUT );
}

value_t AnalogDevice::_read() {

	return analogRead( _pin );
}

AnalogDevice& AnalogDevice::pullup( bool on ) {

	pinMode( _pin, on ? INPUT_PULLUP : INPUT );

	return *this;
}

/*****************************************************************************
*
*   L E V E R  
*
*****************************************************************************/

Lever::Lever( pin_t pin, value_t min, value_t max ) 
		: AnalogDevice( pin )
		, _min( min ), _max( max ) {

	_sum = 0;
	_count = 0;
	_old = 0;

	_samples = 1;
	_rangeMin = min;
	_rangeMax = max;
}

Lever& Lever::average( value_t samples ){

	_samples = samples;

	return *this;
}

Lever& Lever::tranpose( value_t rangeMin, value_t rangeMax ){

	_rangeMin = rangeMin;
	_rangeMax = rangeMax;

	return *this;
}

void Lever::loop(){

	value_t val;

	val = _read();

	_sum += val;
	_count ++;

	if( _samples == 1 ) {

		Serial.print( "=" );
		Serial.print( val );
		Serial.println( "=" );

		_activate( val, _old, 1 );
		_old = val;

	} else if( _count == _samples ) {

		val = _sum / _count;

		_activate( val, _old, _count );

		_count = 0;
		_sum = 0;
		_old = val;
	}
}


/*****************************************************************************
*
*   H A N D L E R
*
*****************************************************************************/

Change::Change( callback_t callback ) 
		: Handler( HT_CHANGE, callback ) {

}

bool Change::handle( Device &dev, value_t newState, value_t oldState, time_t time ) {

	if( newState != oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}

#pragma GCC diagnostic pop
