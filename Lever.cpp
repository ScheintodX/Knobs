#include "Lever.h"

#include <Arduino.h>
// fix f. silly macros
#undef min
#undef max

#include "algorithm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

/*****************************************************************************
*
*   A n a l o g  D e v i c e
*
*****************************************************************************/

AnalogDevice::AnalogDevice( const char *name, pin_t pin )
		: Device( name )
		, _pin( pin ) {

	pinMode( pin, INPUT );
}

knob_value_t AnalogDevice::_read() {

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

Lever::Lever( const char *name, pin_t pin, knob_value_t min, knob_value_t max )
		: AnalogDevice( name, pin )
		, minValue( min ), maxValue( max ) {

	_old = 0;
	_lastTime = 0;
}

Lever& Lever::modify( LeverModifier &modifier ) {

	_modifiers.add( modifier );
	return *this;
}

bool Lever::modify( knob_value_t *val_p ) {

	register bool ok = true;
	register LeverModifier *mod;

	for( mod = _modifiers.first(); mod && ok; mod = _modifiers.next() ) {

		ok = mod->modify( *this, val_p );
	}

	return ok;
}

void Lever::activate( knob_value_t val ) {

	knob_time_t now;

	now = millis();
	_activate( val, _old, now - _lastTime );
	_old = val;
	_lastTime = now;
}

void Lever::loop(){

	knob_value_t val = _read();

	if( modify( &val ) ) {

		activate( val );
	}
}

/*****************************************************************************
*
*   M O D I F I E R
*
*****************************************************************************/

Transpose::Transpose( knob_value_t min, knob_value_t max )
		: _minValue( min )
		, _range( max-min ) {
}

bool Transpose::modify( Lever &lever, knob_value_t *val ) {

	register big_knob_value_t v = *val;
	knob_value_t range = lever.maxValue - lever.minValue;

	v = (v-lever.minValue) * _range / range - _minValue;

	*val = (knob_value_t) v;

	return true;
}

Average::Average( knob_value_t samples )
		: _samples( samples )
		{

	_count = 0;
	_sum = 0;
}


bool Average::modify( Lever &lever, knob_value_t *val ) {

	_count++;
	_sum += *val;

	if( _count == _samples ) {

		*val = _sum / _count;
		_sum = 0;
		_count = 0;
		return true;
	} else {
		return false;
	}
}

AverageTime::AverageTime( knob_time_t time )
		: _time( time )
		{

	_start = 0;
	_sum = 0;
	_count = 0;
}


bool AverageTime::modify( Lever &lever, knob_value_t *val ) {

	knob_time_t now = millis(),
	       span = now - _start;

	_sum += *val;
	_count ++;

	if( span >= _time ) {

		*val = _sum / _count;
		_start = now;
		_sum = 0;
		_count = 0;
		return true;
	} else {
		return false;
	}
}

RunningAverage::RunningAverage( knob_value_t samples )
		: _samples( samples ) {

	_count = 0;
	_avgValue = 0;
}

bool RunningAverage::modify( Lever &lever, knob_value_t *val ) {

	//col( *val );
	//col( _count );

	// Startup: First value is stored
	if( _count == 0 ) _avgValue = *val;

	//col( _avg*100 );

	_avgValue = ( _avgValue * (_samples-1) + *val ) / _samples;

	//col( _avgValue*100 );

	_count++;

	// Onyl return values if min. _samples are collected
	if( _count < _samples ) return false;

	_count = _samples; // prevent overflow

	*val = (knob_value_t)_avgValue;

	//col( *val );

	return true;
}


Deviation::Deviation( knob_time_t time )
		: AverageTime( time ) {

	_minValue = KNOB_VAL_MAX;
	_maxValue = KNOB_VAL_MIN;
}

bool Deviation::modify( Lever &lever, knob_value_t *val ) {

	bool ok;
	knob_value_t tmp = *val;
	knob_value_t delta;

	_minValue = Math::kmin( _minValue, tmp );
	_maxValue = Math::kmax( _maxValue, tmp );

	ok = AverageTime::modify( lever, &tmp );

	if( ok ) {

		delta = ( Math::kabs( _maxValue - tmp ), Math::kabs( tmp - _minValue ) ) / 2;

		*val = delta;

		_minValue = KNOB_VAL_MAX;
		_maxValue = KNOB_VAL_MIN;

		return true;

	} else {
		return false;
	}

}

RunningDeviation::RunningDeviation( knob_value_t samples )
		: RunningAverage( samples ) {

	_minValue = 0;
	_maxValue = 0;
}

bool RunningDeviation::modify( Lever &lever, knob_value_t *val ) {

	knob_value_t avg = *val;

	bool ok = RunningAverage::modify( lever, &avg );

	if( *val < _avgValue ) {

		_minValue = ( _minValue * (_samples-1) + *val ) / _samples;

	} else if( *val > _avgValue ) {

		_maxValue = ( _maxValue * (_samples-1) + *val ) / _samples;

	} else {

		_minValue = ( _minValue * (_samples-1) + *val ) / _samples;
		_maxValue = ( _maxValue * (_samples-1) + *val ) / _samples;
	}

	if( !ok ) return false;

	*val = ( Math::kabs( _minValue ) + Math::kabs( _maxValue ) ) / 2;

	return true;

}


/*****************************************************************************
*
*   H A N D L E R
*
*****************************************************************************/

/*
Change::Change( callback_t callback )
		: Handler( HT_CHANGE, callback ) {

}

bool Change::handle( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t time ) {

	if( newState != oldState )
			return _callback( dev, newState, oldState, time );

	return true;
}
*/

#pragma GCC diagnostic pop
