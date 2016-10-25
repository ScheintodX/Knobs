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

	class Valve {

		friend class Transducer;

		private:
			const pin_t _pin;
			bool _active;
			bool _stored;

			bool _invert;

			Valve& _turn( bool on );

			Valve *_next;

		public:
			Valve( pin_t pin );

			Valve &invert();

			Valve& active( bool on );
			Valve& turn( bool on );
			bool active();
			Valve& on();
			Valve& off();
			bool toggle();

			Valve& store();
			Valve& restore();
	};


	class Transducer {

		private:
			Valve *_first;

			inline Transducer& _turn( bool on );

		public:
			Transducer& operator<<( Valve &valve );

			Transducer& active( bool on );
			Transducer& turn( bool on );
			Transducer& on();
			Transducer& off();
			Transducer& toggle();

			Transducer &store();
			Transducer &restore();
	};

}

#pragma GCC diagnostic pop

#endif
