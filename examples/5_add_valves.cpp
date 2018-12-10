#include <Energia.h>
#include <Knobs.h>

// Valves are outputs we can use

using namespace Knobs;

Knob knob1( PUSH1 ),
     knob2( PUSH2 )
	 ;
Panel panel;

Valve valve1( LED1 ),
      valve2( LED2 )
	  ;

// Define Transducer to organize Valves
Transducer transducer;

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

	panel << knob1 <<< knob2;

	// Setup valves
	valve1.invert( true ) // invert so that logical true
	      .active( false ) // Set starting value
	      ;
    valve2.invert( true )
	      .active( false )
	      ;

	// Put valves in transducer
	transducer << valve1 << valve2;

	panel.begin();
	transducer.begin();
}

void loop() {

	delay( 10 );

    panel.loop();

	// Valves need looping to. But since we have them organized
	// we only need to loop transducer.
	transducer.loop();
}
