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

#define _SEC(n) ((n)*1000)
#define _MS(n) (n)

#define _TP_FIRST_WARNING _SEC( 10 )
#define _TP_SECOND_WARNING _SEC( 3 )
#define _TP_WARNING _MS( 100 )

namespace Knobs {

	enum ValveState {
		OFF=false, ON=true
	};

	class Valve;
	class Transducer;
	class Professor;
	class Buttler;

	typedef void (*transducer_callback_t)( Transducer &t, Valve &valve );

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
			bool _locked : 1;

			Valve *_slave;
			Professor *_owner;
			Buttler *_listener;

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

			Valve& direct( Buttler &listener );

			virtual Valve& active( bool on, bool silent=false );

			bool active();
			Valve& on();
			Valve& off();
			Valve& toggle();

			Valve& store();
			Valve& restore();

			Valve& mute( bool on );
			Valve& unmute();
			bool muted();

			Valve& lock();
			Valve& unlock();
			bool locked();

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

			Transducer& each( transducer_callback_t cb );

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
			virtual void onLoop( Valve &valve, knob_time_t time ) = 0;
			virtual bool onChange( Valve &valve, knob_value_t oldVal, knob_value_t newVal ) = 0;
	};

	class Buttler {

		public:
			virtual void onChange( Valve &valve, knob_value_t oldVal, knob_value_t newVal ) = 0;

	};

	class TimedProfessor : public Professor {

		private:
			const knob_time_t _holdTime;
			knob_time_t _startTime;
			knob_time_t _running;

			knob_time_t _firstWarning;
			knob_time_t _secondWarning;

		public:
			TimedProfessor( knob_time_t holdTime );
			TimedProfessor( knob_time_t holdTime, knob_time_t firstWarning, knob_time_t secondWarning );

			void start();
			void stop();
			void reset();

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
			TimedValve( const char * const name, pin_t pin,
					knob_time_t holdTime, knob_time_t firstWarning, knob_time_t secondWarning );

			void keep();

			TimedProfessor &timer();
	};


}

#pragma GCC diagnostic pop

#endif
