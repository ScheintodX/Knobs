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
	class Professor;

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
		friend class Professor;

		private:
			const char * _name;

			const pin_t _pin;

			bool _invert : 1;
			bool _inputWhenOff : 1;

			bool _active : 1;
			bool _stored : 1;
			bool _mute : 1;

			Valve *_slave;
			Professor *_owner;

			void _init();
			void _turn( bool on );
			void _pinMode( bool to );

			virtual bool _modify( bool on );

			Valve& _print( const char *msg, bool val );

		public:
			Valve( const char * const name, pin_t pin );

			Valve &begin();

			Valve &invert( bool on );
			Valve &inputWhenOff( bool on );

			Valve &enslave( Valve &slave );
			Valve& handover( Professor &owner );

			virtual Valve& active( bool on );

			bool active();
			Valve& on();
			Valve& off();
			Valve& toggle();

			Valve& store();
			Valve& restore();

			Valve& mute( bool on );
			Valve& unmute();
			bool muted();

			pin_t pin();
			const char * name();

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

			Transducer& mute( bool on );
			Transducer& unmute();


			Transducer& print();

			const char * name();

			void loop();
	};

	class Professor {

		public:
			virtual void onLoop( Valve &owner, knob_time_t time );
			virtual bool onChange( Valve &owner, knob_value_t oldVal, knob_value_t newVal );
	};

	class TimedProfessor : public Professor {

		private:
			const knob_time_t _holdTime;
			knob_time_t _startTime;
			knob_time_t _running;

		protected:

			//inline bool _ist( knob_time_t now, knob_time_t back, knob_time_t delta );

		public:
			TimedProfessor( knob_time_t holdTime );

			void start();
			void stop();

		public:
			virtual void onLoop( Valve &owner, knob_time_t time );
			virtual bool onChange( Valve &owner, knob_value_t oldVal, knob_value_t newVal );
	};

	class TimedValve : public Valve {

		friend class TimedProfessor;

		private:
			TimedProfessor _timer;

		public:
			TimedValve( const char * const name, pin_t pin,
					knob_time_t holdTime );

			void keep();
	};


}

#pragma GCC diagnostic pop

#endif
