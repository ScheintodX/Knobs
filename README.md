# Knobs

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

	void setup() {
		knob
			.invert( true )
			.onClick( onClick )
			.onHold( onHold )
			;
	}

	void loop() {

		knob.loop();
	}

