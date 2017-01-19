#ifndef VALVE_H
#define VALVE_H

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#include "knobs_common.h"

namespace Knobs {

	enum ValveState {
		OFF=false, ON=true
	};

	class Transducer;

	/*
	class Switchable {
		virtual Switchable& active( bool on ) = 0;
		virtual bool active() = 0;
		virtual Switchable& on() = 0;
		virtual Switchable& off() = 0;
		virtual bool toggle() = 0;
	};
	*/

	/*
	class Chainable {
		private:
			Chainable *_next;
		public:
			Chainable& prepend( Chainable &next );
	};
	*/

	class Valve {

		friend class Transducer;

		private:
			const pin_t _pin;
			bool _active;
			bool _stored;

			bool _invert;
			bool _inputWhenOff;

			Valve *_next;

		public:
			Valve( pin_t pin );

			Valve &invert( bool on );
			Valve &inputWhenOff( bool on );

			virtual Valve& turn( bool on );

			Valve& active( bool on );
			bool active();
			Valve& on();
			Valve& off();
			bool toggle();

			Valve& store();
			Valve& restore();

			const pin_t pin();
	};

	class DoubleValve : public Valve {

		private:
			Valve &_one;
			Valve &_two;

		public: 
			DoubleValve( Valve &one, Valve &two );

			virtual Valve& turn( bool on );
	};

	class Transducer {

		private:
			Valve *_first;

		public:

			Transducer();
			Transducer( Valve &v1 );
			Transducer( Valve &v1, Valve &v2 );
			Transducer( Valve &v1, Valve &v2, Valve &v3 );
			Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4 );
			Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5 );
			Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6 );
			Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6, Valve &v7 );
			Transducer( Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6, Valve &v7, Valve &v8 );

			Transducer& operator<<( Valve &valve );

			Transducer& active( bool on );
			Transducer& turn( bool on );
			Transducer& on();
			Transducer& off();
			Transducer& toggle();

			Transducer& activeMask( uint32_t mask );

			Transducer &store();
			Transducer &restore();
	};

}

#pragma GCC diagnostic pop

#endif
