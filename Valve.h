#ifndef VALVE_H
#define VALVE_H

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#include "knobs_common.h"
#include "Canister.h"
#include "algorithm.h"

#ifndef KNOBS_TRANSDUCER_CANISTER_SIZE
	#define KNOBS_TRANSDUCER_CANISTER_SIZE 20
#endif

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
	class Fireman {
		virtual void loop
	}
	*/


	class Valve {

		friend class Transducer;
		friend class Fireman;

		private:
			const char * _name;

			const pin_t _pin;
			bool _active;
			bool _stored;

			bool _invert;
			bool _inputWhenOff;

			bool _isInit;

			Valve *_slave;

			void _init();
			void _pinMode( bool to );

			virtual bool _modify( bool on );

			Valve& _print( const char *msg, bool val );

		public:
			Valve( const char * const name, pin_t pin );

			Valve &begin();

			Valve &invert( bool on );
			Valve &inputWhenOff( bool on );

			Valve &enslave( Valve &slave );

			virtual Valve& active( bool on );

			bool active();
			Valve& on();
			Valve& off();
			Valve& toggle();

			Valve& store();
			Valve& restore();

			pin_t pin();
			const char * name();

			virtual void loop( knob_time_t time );
	};


	class TimedValve : public Valve {

		private:
			Valve &_nested;
			const knob_time_t _holdTime;
			const knob_time_t _notifyTime;
			knob_time_t _holdUntil;
			knob_time_t _notifyAt;
			knob_time_t _active;

		protected:

			void _start();

		public:
			TimedValve( const char * const name, Valve &nested,
					knob_time_t holdTime, knob_time_t notifyTime );

			virtual Valve& active( bool on );

			TimedValve& keep();

			virtual void loop( knob_time_t time );
	};


	class Transducer {

		private:
			const char *_name;
			Canister<Valve, KNOBS_TRANSDUCER_CANISTER_SIZE> _valves;

		public:

			Transducer( const char *name );
			Transducer( const char *name, Valve &v1 );
			Transducer( const char *name, Valve &v1, Valve &v2 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3, Valve &v4 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6, Valve &v7 );
			Transducer( const char *name, Valve &v1, Valve &v2, Valve &v3, Valve &v4, Valve &v5,
					Valve &v6, Valve &v7, Valve &v8 );

			Transducer& operator<<( Valve &valve );

			Transducer& begin();

			Transducer& active( bool on );
			Transducer& on();
			Transducer& off();
			Transducer& toggle();

			Transducer& activeMask( uint32_t mask );
			uint32_t activeMask();

			Transducer& store();
			Transducer& restore();

			Transducer& print();

			const char * name();

			void loop();
	};

}

#pragma GCC diagnostic pop

#endif
