# BulliBus

=== JUST NOT READY, YET! ===
(but let's call it *beta*)

Library for easy plug&play use of buttons and switches (and more to come)

"It seems that perfection is attained, not when there is nothing more to add,
but when there is nothing more to take away."
                                                    (Antoine de Saint Exupéry)

TLDR: Just look in the examples folder.

The purpose of this project is to provide an extremely simple to use but
nevertheless powerfull library which simplifies the building of hardware
user-interfaces.

With this library you can define events on input pins. There can be multiple
event on each input pin. So you can e.g. check for click and hold at the same
time.

EXAMPLE:

	bool onClick( ... ){
		Serial.println( "CLICK" );
		return true;
	}

	bool onHold( ... ){
		Serial.println( "CLICK" );
		return true;
	}

    Knob knob( D1 );
	Click click = Click( onClick );
	Hold hold = Hold( onHold, 500 );

	void setup() {
		knob
			.on( click )
			.on( hold )
			;
	}

	void loop() {

		knob.loop();
	}


ISSUES:
 * Not ready yet
 * Make even more simple to use
