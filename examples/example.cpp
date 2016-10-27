#include <Energia.h>
#include <Knob.h>
#include <Valve.h>

using namespace Knobs;

Knob knob1( PUSH1 ), knob2( PUSH2 );
Panel panel;

void onRelease() {
    Serial.print( "*release*" );
}

void onHold() {
    Serial.print( "*hold*" );
}

void onLonghold() {
    Serial.print( "*long*" );
}

void onClick() {
    Serial.print( "*click*" );
}

void onDoubleClick() {
	Serial.print( "*doubleclick*" );
}

bool onPush( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

	if( &dev == &knob1 )
			Serial.print( "*Knob1" );
	if( &dev == &knob2 )
			Serial.print( "*Knob2" );

    Serial.print( " :push*" );
    return true;
}

Push push = Push( onPush );
Release release = Release( onRelease );
Hold hold = Hold( onHold, 200 ),
     holdX = Hold( onLonghold, 500 )
     ;
Click click( onClick );
DoubleClick doubleClick( onDoubleClick );

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

    knob1.pullup( true )
      .invert( true )
      .on( push )
      .on( release )
      .on( hold ).on( holdX )
      .on( click )
	  .on( doubleClick )
      ;
    knob2.pullup( true )
      .invert( true )
	  .on( push )
      ;

    panel << knob2 << knob1;

}

void loop() {

	delay( 10 );
    panel.loop();
}
