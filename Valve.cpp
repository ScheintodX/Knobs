#include "Valve.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

Valve::Valve( pin_t pin ) 
		: _pin( pin ){

	_active = false;
	_invert = false;
	_saved = false;

	pinMode( pin, OUTPUT );
}

Valve& Valve::invert() {
	_invert = !_invert;
	return *this;
}

inline Valve& Valve::_turn( bool on ) {
	_active = on;
	digitalWrite( _pin, _active ^ _invert );
	return *this;
}

Valve& Valve::active( bool on ) {
	return _turn( on );
}

Valve& Valve::turn( bool on ) {
	return _turn( on );
}

Valve& Valve::on() {
	return _turn( true );
}
Valve& Valve::off() {
	return _turn( false );
}

bool Valve::active() {

	return _active;
}

bool Valve::toggle() {

	_turn( !_active );
	return _active;
}


Valve& Valve::save() {
	_saved = _active;
	return *this;
}
Valve& Valve::restore() {
	_active = _saved;
	return _turn( _active );
}



/*
 * T R A N S D U C E R
 */

Transducer& Transducer::operator <<( Valve &valve ) {

	valve._next = _first;
	_first = &valve;
	return *this;
}

#define TONALL( m ) \
			Valve *val; \
			for( val=_first; val; val = val->_next ) { \
					val->m(); \
			}
#define TONALLP( m, p ) \
			Valve *val; \
			for( val=_first; val; val = val->_next ) { \
					val->m( p ); \
			}

inline Transducer& Transducer::_turn( bool on ) {

	TONALLP( turn, on );
	return *this;
}
Transducer& Transducer::turn( bool on ) {
	return _turn( on );
}
Transducer& Transducer::active( bool on ) {
	return _turn( on );
}
Transducer& Transducer::on() {
	return _turn( ON );
}
Transducer& Transducer::off() {
	return _turn( OFF );
}
Transducer& Transducer::toggle() {
	TONALL( toggle );
	return *this;
}
Transducer& Transducer::save() {
	TONALL( save );
	return *this;
}
Transducer& Transducer::restore() {
	TONALL( restore );
	return *this;
}
#pragma GCC diagnostic pop
