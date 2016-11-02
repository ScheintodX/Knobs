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

			Valve *_next;

		public:
			Valve( pin_t pin );

			Valve &invert();

			Valve& active( bool on );
			virtual Valve& turn( bool on );
			bool active();
			Valve& on();
			Valve& off();
			bool toggle();

			Valve& store();
			Valve& restore();
	};

	class DoubleValve : public Valve {

		private:
			Valve &_other;

		public: 
			DoubleValve( pin_t pin, Valve &other );

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

			Transducer &store();
			Transducer &restore();
	};

}

#pragma GCC diagnostic pop

#endif
