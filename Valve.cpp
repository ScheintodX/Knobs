#include "Valve.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

Valve::Valve( const char * const name, pin_t pin ) : _name( name ), _pin( pin ) {

	_active = false;
	_invert = false;
	_stored = false;
	_inputWhenOff = false;
	_isInit = false;
}

Valve& Valve::begin(){
	_init();
	return *this;
}

void Valve::_init() {

	if( _inputWhenOff ) {
		_pinMode( _active );
	} else {
		pinMode( _pin, OUTPUT );
	}

	_isInit = true;
}

void Valve::_pinMode( bool to ) {

	if( to ) pinMode( _pin, OUTPUT );
	else pinMode( _pin, INPUT );
}

bool Valve::_modify( bool on ) {

	return _invert ? !on : on;
}

Valve& Valve::invert( bool on ) {
	_invert = on;
	return *this;
}
Valve& Valve::inputWhenOff( bool on ) {
	_inputWhenOff = on;
	return *this;
}
Valve& Valve::enslave( Valve &slave ) {
	_slave = &slave;
	return *this;
}

Valve& Valve::active( bool on ) {

	bool to = _modify( on );

	_active = on;

	if( _inputWhenOff ) _pinMode( to );

	//_print( "turn", to );

	digitalWrite( _pin, to );

	if( _slave ) _slave->active( on );

	return *this;
}

Valve& Valve::on() {
	return active( true );
}
Valve& Valve::off() {
	return active( false );
}

bool Valve::active() {

	return _active;
}

Valve& Valve::toggle() {

	active( !_active );
	return *this;
}


Valve& Valve::store() {
	_stored = _active;
	return *this;
}
Valve& Valve::restore() {
	_active = _stored;
	return active( _active );
}

pin_t Valve::pin() {
	return _pin;
}
const char * Valve::name() {
	return _name;
}

void Valve::loop( knob_time_t time ) {}

Valve& Valve::_print( const char *msg, bool val ) {

	Serial.print( msg );
	Serial.print( " " );
	Serial.print( _name );
	Serial.print( "(" );
	Serial.print( _pin );
	Serial.print( "/" );
	Serial.print( _invert );
	Serial.print( "> " );
	Serial.println( val );

	return *this;
}

/*
 * T I M E D  V A L V E
 */

TimedValve::TimedValve( const char * const name, Valve &nested,
		knob_time_t holdTime, knob_time_t notifyTime ) 
		: Valve( name, 0 )
		, _nested( nested )
		, _holdTime( holdTime )
		, _notifyTime( notifyTime )
		{
		
	_active = false;
}

void TimedValve::_start() {

	knob_time_t now = millis();

	_holdUntil = now + _holdTime;
	_notifyAt = now + _notifyTime;
	_active = true;
}

void TimedValve::loop( knob_time_t time ) {

	if( !_active ) return;

}

TimedValve& TimedValve::keep() {

	_active = false;

	return *this;
}

Valve& TimedValve::active( bool on ) {

	_nested.active( on );

	return *this;
}


/*
 * T R A N S D U C E R
 */

Transducer::Transducer( const char *name )
		: _name( name ) {}
Transducer::Transducer( const char *name, Valve &v1 )
		: _name( name ) {
	*this << v1;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2 )
		: _name( name ) {
	*this << v1 << v2;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3 )
		: _name( name ) {
	*this << v1 << v2 << v3;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3,
		Valve &v4 )
		: _name( name ) {
	*this << v1 << v2 << v3 << v4;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3,
		Valve &v4, Valve &v5 )
		: _name( name ) {
	*this << v1 << v2 << v3 << v4 << v5;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3,
		Valve &v4, Valve &v5, Valve &v6 )
		: _name( name ) {
	*this << v1 << v2 << v3 << v4 << v5 << v6;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3,
		Valve &v4, Valve &v5, Valve &v6, Valve &v7 )
		: _name( name ) {
	*this << v1 << v2 << v3 << v4 << v5 << v6 << v7;
}
Transducer::Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3,
		Valve &v4, Valve &v5, Valve &v6, Valve &v7, Valve &v8 )
		: _name( name ) {
	*this << v1 << v2 << v3 << v4 << v5 << v6 << v7 << v8;
}

Transducer& Transducer::operator <<( Valve &valve ) {

	_valves.add( valve );
	return *this;
}

#define TONALL( m ) \
			Valve *valve; \
			for( valve=_valves.first(); valve; valve=_valves.next() ) { \
					valve->m(); \
			}
#define TONALLP( m, p ) \
			Valve *valve; \
			for( valve=_valves.first(); valve; valve=_valves.next() ) { \
					valve->m( p ); \
			}

Transducer& Transducer::begin() {
	TONALL( begin );
	return *this;
}
inline Transducer& Transducer::active( bool on ) {

	TONALLP( active, on );
	return *this;
}
Transducer& Transducer::on() {
	return active( ON );
}
Transducer& Transducer::off() {
	return active( OFF );
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
Transducer& Transducer::print() {

	Serial.print( "| " );
	Valve *valve; \
	for( valve=_valves.first(); valve; valve=_valves.next() ) { 
		Serial.print( valve->active() ? "X" : "o" );
		Serial.print( " " );
	}
	Serial.println( "|" );

	return *this;
}
Transducer& Transducer::activeMask( uint32_t mask ) {

	Valve *valve;
	int count = 1;

	for( valve=_valves.first(); valve; valve=_valves.next() ) { 

		valve->active( mask & count );
		count = count<<1;
	}
	return *this;
}
uint32_t Transducer::activeMask() {

	uint32_t mask = 0;
	int count = 1;
	Valve *valve;

	for( valve=_valves.first(); valve; valve=_valves.next() ) { 

		mask |= valve->active() ? count : 0;
		count = count<<1;
	}
	return mask;
}

const char * Transducer::name() {
	return _name;
}

void Transducer::loop() {

	Valve *valve;
	knob_time_t now = millis();

	for( valve = _valves.first(); valve; valve = _valves.next() ) {

		valve->loop( now );
	}
}
#pragma GCC diagnostic pop
