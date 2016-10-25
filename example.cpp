#include <Energia.h>
#include <Knob.h>
#include <Valve.h>

using namespace Knobs;

bool onPush( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

    Serial.print( "*push*" );
    return true;
}

bool onRelease( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

    Serial.print( "*release*" );
    return true;
}

bool onHold( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

    Serial.print( "*hold*" );
    return true;
}
bool onLonghold( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

    Serial.print( "*long*" );
    return true;
}
void onClick() {

    Serial.print( "*click*" );
}

Knob b1( PUSH1 ), b2( PUSH2 );
Panel panel;
Push p = Push( onPush );
Release r = Release( onRelease );
Hold h = Hold( onHold, 200 ),
     hX = Hold( onLonghold, 500 )
     ;
Click click( onClick, 250 );

Valve light[] = {
	Valve( D1_LED ),
	Valve( D2_LED ),
	Valve( D3_LED ),
	Valve( D4_LED )
};

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );
    Serial.print( "time_t is " );
    Serial.println( sizeof( time_t ) );

    b1.pullup( true )
      .invert( true )
      .on( p )
      .on( r )
      .on( h ).on( hX )
      .on( click )
      ;
    b2.pullup( true )
      .invert( true )
      ;

    panel << b2 << b1;

	light[ 1 ].turn( ON );
	light[ 2 ].turn( OFF );
	light[ 3 ].invert();
}

void loop() {

	light[ 0 ].toggle();
	light[ 1 ].toggle();
	light[ 2 ].toggle();
	light[ 3 ].toggle();
	delay( 100 );
    panel.loop();
}
