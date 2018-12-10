#include <Energia.h>
#include <Knobs.h>

// Define multiple Knobs and organize them
// using Panels

using namespace Knobs;

Knob knob1( PUSH1 ),
     knob2( PUSH2 )
	 ;

// Define Panel to organize Knobs
Panel panel;

void callbackClick1() {
    Serial.print( "*click1*" );
}
void callbackClick2() {
    Serial.print( "*click2*" );
}

void setup() {

    Serial.begin( 19200 );
    Serial.println( "Hello" );

    knob1.onClick( callbackClick1 )
         ;
    knob2.onClick( callbackClick2 )
         ;

	// add all knobs to panel
	panel << knob1 <<< knob2;

	// we only need to call begin once.
	panel.begin();
}

void loop() {

	delay( 10 );

	// We can now use panel to loop.
	// This one calls every knob's loop().
    panel.loop();
}
