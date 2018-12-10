#include <Energia.h>
#include <Knob.h>
#include <Valve.h>

using namespace Knobs;

Knob knob1( PUSH1 ), knob2( PUSH2 );
Panel panel;

void callbackRelease() {
    Serial.print( "*release*" );
}

void callbackHold() {
    Serial.print( "*hold*" );
}

void callbackLonghold() {
    Serial.print( "*long*" );
}

void callbackClick() {
    Serial.print( "*click*" );
}

void onDoubleClick() {
	Serial.print( "*doubleclick*" );
}

bool callbackPush( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) {

	if( &dev == &knob1 )
			Serial.print( "*Knob1" );
	if( &dev == &knob2 )
			Serial.print( "*Knob2" );

    Serial.print( " :push*" );
    return true;
}

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

    knob1.pullup( true )
      .invert( true )
      .onPush( callbackPush )
      .onRelease( callbackRelease )
      .onHold( callbackHold, 200 ).onHold( callbackLongHold, 500 )
      .callbackClick( callbackClick )
	  .onDoubleClick( onDoubleClick )
      ;

    knob2.pullup( true )
      .invert( true )
	  .onPush( callbackPush )
      ;

    panel << knob2 << knob1;

}

void loop() {

	delay( 10 );
    panel.loop();
}
