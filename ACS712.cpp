#include <Arduino.h>
#include "ACS712.h"
#include <E.h>
#include <algorithm>

// Measured using 3.3v logic

using namespace Knobs;

static const float _MV_PER_DIV = 5000.0/2048.0;
static const float _ACS_SENS[] = { 0.185, 0.100, 0.066 }; //mV/mA s. datasheet
static const float _ACS_MUL[] = {
		_MV_PER_DIV/_ACS_SENS[ x05B ],
		_MV_PER_DIV/_ACS_SENS[ x20A ],
		_MV_PER_DIV/_ACS_SENS[ x30A ]
};

ACS712::ACS712( pin_t pin, ACS_VERSION version,
		knob_value_t max, knob_value_t samples ) 
		: Lever( pin, 0, max )
		, _scale( _ACS_MUL[ version ] )
		, _samples( samples )
		{

	_div_avg = 0;
	_val_avg = 0;
	_count = 0;
}

void ACS712::loop(){

	knob_value_t in, val;
	float div;

	in = _read();

	_count++;

	if( _count == 1 ) {
		_val_avg = in;
		return;
	}

	_val_avg = ( _val_avg * (_samples-1) + in ) / _samples;

	if( _count < _samples ) return;

	div = fabs( _val_avg - in );

	if( _count == _samples ) {
		_div_avg = div;
	}

	_div_avg = ( _div_avg * (_samples-1) + div ) / _samples;

	if( _count < _samples*2 ) return;
	_count = _samples*2; //prevent overflow

	val = (knob_value_t) ( _div_avg * _ACS_MUL[ 1 ] );

	if( modify( &val ) ) {

		activate( val );
	}

}
