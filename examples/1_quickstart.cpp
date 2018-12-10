#include <Energia.h>
#include <Knobs.h>

// Quickstart using Knobs with one simple click handler
// A 'click' with Knobs is pushing and releasing the pin.

using namespace Knobs;

// Define one Knob on PIN PUSH1 (defined elsewhere)
Knob knob( PUSH1 );

// Define one callback which is called on click
void callbackClick() {
    Serial.print( "*click*" );
}

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

	// Setup knob to call callback on click
    knob.onClick( callbackClick )
        ;

	// start operation
	knob.begin();
}

void loop() {

	// make sure loop is called frequently or the knobs get laggy
	delay( 10 );

	// call knob's loop. From here callbacks are called
    knob.loop();
}
