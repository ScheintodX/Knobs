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
	_stored = false;

	pinMode( pin, OUTPUT );
}

Valve& Valve::invert() {
	_invert = !_invert;
	return *this;
}

Valve& Valve::turn( bool on ) {
	_active = on;
	digitalWrite( _pin, _active ^ _invert );
	return *this;
}

Valve& Valve::active( bool on ) {
	return turn( on );
}

Valve& Valve::on() {
	return turn( true );
}
Valve& Valve::off() {
	return turn( false );
}

bool Valve::active() {

	return _active;
}

bool Valve::toggle() {

	turn( !_active );
	return _active;
}


Valve& Valve::store() {
	_stored = _active;
	return *this;
}
Valve& Valve::restore() {
	_active = _stored;
	return turn( _active );
}

/*
 * D O U B L E  V A L V E
 */

DoubleValve::DoubleValve( pin_t pin, Valve &other )
		: Valve( pin )
		, _other( other )
		{}

Valve& DoubleValve::turn( bool on ) {

	_other.turn( on );
	return Valve::turn( on );
}


/*
 * T R A N S D U C E R
 */

Transducer::Transducer(){}
Transducer::Transducer( Valve &v1 ){
	*this << v1;
}
Transducer::Transducer( Valve &v1, Valve &v2 ){
	*this << v1 << v2;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3 ){
	*this << v1 << v2 << v3;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4 ){
	*this << v1 << v2 << v3 << v4;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5 ){
	*this << v1 << v2 << v3 << v4 << v5;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
		Valve &v6 ){
	*this << v1 << v2 << v3 << v4 << v5 << v6;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
		Valve &v6, Valve &v7 ){
	*this << v1 << v2 << v3 << v4 << v5 << v6 << v7;
}
Transducer::Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
		Valve &v6, Valve &v7, Valve &v8 ){
	*this << v1 << v2 << v3 << v4 << v5 << v6 << v7 << v8;
}

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

inline Transducer& Transducer::turn( bool on ) {

	TONALLP( turn, on );
	return *this;
}
Transducer& Transducer::active( bool on ) {
	return turn( on );
}
Transducer& Transducer::on() {
	return turn( ON );
}
Transducer& Transducer::off() {
	return turn( OFF );
}
Transducer& Transducer::toggle() {
	TONALL( toggle );
	return *this;
}
Transducer& Transducer::store() {
	TONALL( store );
	return *this;
}
Transducer& Transducer::restore() {
	TONALL( restore );
	return *this;
}

#pragma GCC diagnostic pop
