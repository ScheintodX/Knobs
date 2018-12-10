#include <Energia.h>
#include <Knobs.h>

// Add pin configuration

using namespace Knobs;

Knob knob( PUSH1 );

void callbackClick() {
    Serial.print( "*click*" );
}

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

    knob.pullup( true ) // Activate pin's pullup
		.invert( true ) // Invert logic. Pins logical 1 is knobs logical zero
	    .onClick( callbackClick )
        ;

	knob.begin();
}

void loop() {

	delay( 10 );

    knob.loop();
}
