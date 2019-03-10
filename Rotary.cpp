#include "Rotary.h"
#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

Rotary *Rotary::_POOL[ _NUM_ROT ] = {};
uint8_t Rotary::_IDX = 0;


Rotary::Rotary( const char *name, const pin_t p1, const pin_t p2, uint8_t div )
            : Device( name )
            , _p1(p1)
            , _p2(p2)
            , _idx( _IDX++ )
            , _div( div ) {

    // Note: can be simplified for div==2
    // With only half of the interrupts.

    _value = 0;
    _lastValue = 0;

    if (_idx >= _NUM_ROT) {

        Serial.println("To many rotaries");
    } else {
        _POOL[_idx] = this;
    }

    _start();

}

Rotary& Rotary::_start(){

    switch ( _idx ) {
    case 0:
        // Cannot attach two functions to one pin (>.<)
        attachInterrupt( _p1, _0_1_c, CHANGE );
        attachInterrupt( _p2, _0_2_c, CHANGE );
        break;
    case 1:
        attachInterrupt( _p1, _1_1_c, CHANGE );
        attachInterrupt( _p2, _1_2_c, CHANGE );
        break;
    case 2:
        attachInterrupt( _p1, _2_1_c, CHANGE );
        attachInterrupt( _p2, _2_2_c, CHANGE );
        break;
    case 3:
        attachInterrupt( _p1, _3_1_c, CHANGE );
        attachInterrupt( _p2, _3_2_c, CHANGE );
        break;
    case 4:
        attachInterrupt( _p1, _4_1_c, CHANGE );
        attachInterrupt( _p2, _4_2_c, CHANGE );
        break;
    case 5:
        attachInterrupt( _p1, _5_1_c, CHANGE );
        attachInterrupt( _p2, _5_2_c, CHANGE );
        break;
    case 6:
        attachInterrupt( _p1, _6_1_c, CHANGE );
        attachInterrupt( _p2, _6_2_c, CHANGE );
        break;
    case 7:
        attachInterrupt( _p1, _7_1_c, CHANGE );
        attachInterrupt( _p2, _7_2_c, CHANGE );
        break;
    default:
        Serial.println( "To many rotaries" );
        return *this;
    }

    pinMode(_p1, INPUT_PULLUP);
    pinMode(_p2, INPUT_PULLUP);
    _v1 = digitalRead(_p1) ? +1 : -1;
    _v2 = digitalRead(_p2) ? +1 : -1;

    return *this;
}

int32_t Rotary::value(){
    return _read();
}

void Rotary::loop(){

	if( mute() ) return;

	knob_time_t now = millis();
	knob_value_t value = _read();

    _activate( value, _lastValue, now );

    _lastValue = value;
}

void Rotary::_c1(){
    digitalRead( _p1 ) ? _r1() : _f1();
}
void Rotary::_r1() {
    // the 'ifs' are
    // needed for debouncing
    if( _v1 != 1 ) {
        _v1 = 1;
        _value += _v2;
    }
}
void Rotary::_f1() {
    if( _v1 != -1 ) {
        _v1 = -1;
        _value -= _v2;
    }
}
void Rotary::_c2(){
    digitalRead( _p2 ) ? _r2() : _f2();
}
void Rotary::_r2() {
    if( _v2 != 1 ) {
        _v2 = 1;
        _value -= _v1;
    }
}
void Rotary::_f2() {
    if( _v2 != -1 ) {
        _v2 = -1;
        _value += _v1;
    }
}


DelayedChoise::DelayedChoise( callback_t callback, knob_time_t delay )
        : Handler( HT_DELAYED_CHOISE, callback )
        , _delay( delay ){

    _lastTime = 0;
    _sum = 0;
}

DelayedChoise::DelayedChoise( minimal_callback_t callback, knob_time_t delay )
        : Handler( HT_DELAYED_CHOISE, callback )
        , _delay( delay ){

    _lastTime = 0;
    _sum = 0;
}

DelayedChoise::DelayedChoise( Callable &callable, knob_time_t delay )
        : Handler( HT_DELAYED_CHOISE, callable )
        , _delay( delay ){

    _lastTime = 0;
    _sum = 0;
}

bool DelayedChoise::handle( Device &dev,
        knob_value_t newState, knob_value_t oldState, knob_time_t time ){

    knob_value_t div = newState - oldState;
    bool result;

    if( div ){
        _sum += div;
        _lastTime = time;
    } else if( _sum ) {
        if( time - _lastTime > _delay ) {
			result = _callback( dev, _sum, 0, time );
            _sum = 0;
            return result;
        }
    }
    return false;
}