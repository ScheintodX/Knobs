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

	typedef void (*transducer_callback_t)( Transducer &t, Valve &valve, knob_value_t );

	typedef uint32_t (*t_rotate_f)( uint32_t mask );

	// A Valve is a binary output on one pin
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
			// Initialize with name and pin
			Valve( const char * const name, pin_t pin );

			// Start operation
			Valve &begin();

			// Logical invert output
			Valve &invert( bool on );
			// Turn pin to input when set to false
			Valve &inputWhenOff( bool on );

			// start controlling other valve when this on is operated
			Valve &enslave( Valve &slave );
			// set Professor who can do operation automatically
			Valve& handover( Professor &owner );

			// set Buttler which can react to changes
			Valve& direct( Buttler &listener );

			// switch valve on/off. If called silently buttlers aren't notified
			virtual Valve& active( bool on, bool silent=false );
			// return state
			bool active();
			// turn on
			Valve& on() { return active( true ); }
			// turn off
			Valve& off() { return active( false ); }
			// toggle on/off
			Valve& toggle(){ active( !_active ); return *this; }

			// store current state
			Valve& store();
			// restore last stored state
			Valve& restore();

			// pause operation and keep in 'on' state
			Valve& mute( bool on );
			// resume operation
			Valve& unmute();
			// return mute state
			bool muted();

			// protect current state from change
			Valve& lock();
			// remove protection
			Valve& unlock();
			// return lock state
			bool locked();

			// return pin
			pin_t pin();
			// return name
			const char * name();

			// call periodically in main loop. calls should be every 50ms or quicker
			virtual void loop( knob_time_t time );
	};

	// Transducers combine multiple Valves
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

			// Add another Valve
			Transducer& operator<<( Valve &valve );

			// Start operation. (calls Valves' begin)
			Transducer& begin();

			// Turn on/off all Valves
			Transducer& active( bool on );
			// Turn on all Valves
			Transducer& on();
			// Turn off all Valves
			Transducer& off();
			// Toggle all Valves
			Transducer& toggle();

			// Iterate all Valves and call cb on them
			Transducer& each( transducer_callback_t cb, knob_value_t val=0 );
			// Find one valve by prefix
			Valve* find( const char *prefix );

			// Set valve state my mask
			Transducer& activeMask( uint32_t mask );
			// Return all valves state in mask
			uint32_t activeMask();

			// change Valves' state. Function 'rot' determines how.
			Transducer& rotate( t_rotate_f rot );
			// toggle all valves. If all are off when calling then 'rot' determines which ones to turn on
			Transducer& toggle( t_rotate_f rot );

			// store all Valves' states (calls Valves' store)
			Transducer& store();
			// restore all Valves' states (calls Valves' restore)
			Transducer& restore();

			// mute all Valves. (calls Valves' mute)
			Transducer& mute( bool on );
			// unmute all Valves. (calls Valves' unmute)
			Transducer& unmute();

			// return amount of Valves
			int fill();

			// debug print state of all valves
			Transducer& print();

			// return transducer's name
			const char * name();

			// call periodically in main loop. calls should be every 50ms or quicker
			void loop();
	};

	// Operation can be handed over to an (Mad) Scientist
	// Professor's methods are called on looping and on change
	// He is able to change the outcome of operations
	class Professor {

		public:
			virtual void onLoop( Valve &valve, knob_time_t time ) = 0;
			virtual bool onChange( Valve &valve, knob_value_t oldVal, knob_value_t newVal ) = 0;
	};

	// A Buttler can look at the Valve and to stuff when it's
	// state changes. But he can't alter the Valve's state
	class Buttler {

		public:
			virtual void onChange( Valve &valve, knob_value_t oldVal, knob_value_t newVal ) = 0;

	};

	// "I'm too late"
	// The TimedProfessor switches the Valve off after a certain time
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

	// TimedValve is operated by a TimedProfessor and automagically turns off after
	// a certain time.
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
