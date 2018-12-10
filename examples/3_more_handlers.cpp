#include <Energia.h>
#include <Knobs.h>

// Add more handlers for additional pin functions
// Possible handlers are:
// * Click
// * Hold
// * Release
// * Push

using namespace Knobs;

Knob knob( PUSH1 );

void callbackClick() {
    Serial.print( "*click*" );
}
void callbackPush() {
    Serial.print( "*push*" );
}
void callbackHold() {
    Serial.print( "*hold*" );
}
void callbackRelease() {
    Serial.print( "*release*" );
}

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

    knob.onPush( callbackPush ) // Press buton
		.onHold( callbackHold, 500 ) // Hold for at least 500ms
	    .onClick( callbackClick )
		.onRelease( callbackRelease ) // Release button
        ;

	knob.begin();
}

void loop() {

	delay( 10 );

    knob.loop();
}
