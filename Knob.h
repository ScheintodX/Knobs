#ifndef KNOB_H
#define KNOB_H

#include <stdint.h>
#include <climits>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#include "knobs_common.h"
#include "Canister.h"

#ifndef KNOBS_HANDLER_CANISTER_SIZE 
	#define KNOBS_HANDLER_CANISTER_SIZE 5
#endif
#ifndef KNOBS_PANEL_CANISTER_SIZE 
	#define KNOBS_PANEL_CANISTER_SIZE 20
#endif


/**
 * Everything is put in namespace Knobs in order to avoid conflicts
 */
namespace Knobs {

	#define KNOB_VAL_MAX INT_MAX
	#define KNOB_VAL_MIN INT_MIN

	class Device;
	class Handler;
	class Panel;

	typedef bool (*minimal_callback_t)( knob_value_t val );
	typedef bool (*callback_t)( Device &dev, Handler &handler,
			knob_value_t newState, knob_value_t oldState, knob_time_t count );

	enum HandlerType {
		HT_ALWAYS=0,
		HT_PUSH=1, HT_ACTIVATE=1,
		HT_RELEASE=2, HT_DEACTIVATE=2,
		HT_HOLD=3,
		HT_CLICK=4,
		HT_TOGGLE=5,
		HT_DOUBLECLICK=6,
		HT_MULTICLICK=7,
		HT_CHANGE=8,
		HT_RISE=9,
		HT_FALL=10,
		HT_OVER=11,
		HT_UNDER=12,
		HT_HYSTERESIS=13
	};


	/**
	 * A handler encapsulates on type of action which is looked for in order to do something.
	 * Examples are: push, click, hold, etc...
	 */
	class Handler {

		friend class Device;

		protected:

			const callback_t _cb;
			const minimal_callback_t _cbm;

			virtual bool _callback( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:

			const HandlerType type;

			Handler( HandlerType type, minimal_callback_t callback );
			Handler( HandlerType type, callback_t callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) = 0;

	};


	class Always : public Handler {

		public:

			Always( callback_t callback );
			Always( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};


	class Push : public Handler {

		public:

			Push( callback_t callback );
			Push( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};
	typedef Push Activate;


	class Release : public Handler {

		public:

			Release( callback_t callback );
			Release( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};
	typedef Release Deactivate;


	class Toggle : public Handler {

		public:
			Toggle( callback_t callback );
			Toggle( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};


	class Click : public Handler {

		static const knob_time_t MAX_TIME_CLICK = 0;

		private:
			const knob_time_t _maxTimeClick;
			knob_time_t _timeStart;

		protected:
			Click( HandlerType type, callback_t callback,
					knob_time_t maxTimeClick = MAX_TIME_CLICK );
			Click( HandlerType type, minimal_callback_t callback,
					knob_time_t maxTimeClick = MAX_TIME_CLICK );

		public:
			Click( callback_t callback, knob_time_t maxTime = MAX_TIME_CLICK );
			Click( minimal_callback_t callback, knob_time_t maxTime = MAX_TIME_CLICK );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};


	class DoubleClick : public Click {

		private:

			static const knob_time_t MAX_TIME_CLICK = 0;
			static const knob_time_t MAX_TIME_INBETWEEN = 750;

			const knob_time_t _maxTimeInbetween;
			const uint8_t _maxClicks;

			knob_time_t _timeStartSequence;
			knob_value_t _clicks;

		protected:

			virtual bool _callback( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			DoubleClick( callback_t callback,
					knob_value_t maxClicks = 2,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);
			DoubleClick( minimal_callback_t callback,
					knob_value_t maxClicks = 2,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	class MultiClick : public Click {

		private:

			static const knob_time_t MAX_TIME_CLICK = 250;
			static const knob_time_t MAX_TIME_INBETWEEN = 750;

			const knob_time_t _maxTimeInbetween;

			knob_time_t _timeLastClick;
			knob_value_t _clicks;

		protected:

			virtual bool _callback( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			MultiClick( callback_t callback,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);
			MultiClick( minimal_callback_t callback,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};


	class Hold : public Handler {

		private:
			const knob_time_t _timeHold;

			bool _continues;
			bool _hasSent;

		public:

			Hold( callback_t callback, knob_time_t time );
			Hold( minimal_callback_t callback, knob_time_t time );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

			Hold& continues( bool on );

	};

	class Over : public Handler {

		private:
			const knob_value_t _val;

		public:
			Over( callback_t callback, knob_value_t val );
			Over( minimal_callback_t callback, knob_value_t val );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	class Under : public Handler {

		private:
			const knob_value_t _val;

		public:
			Under( callback_t callback, knob_value_t val );
			Under( minimal_callback_t callback, knob_value_t val );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	class Hysteresis : public Handler {

		private:
			const knob_value_t _upper_bound;
			const knob_value_t _lower_bound;

		public:
			Hysteresis( callback_t callback,
					knob_value_t lower_bound, knob_value_t upper_bound );
			Hysteresis( minimal_callback_t callback,
					knob_value_t lower_bound, knob_value_t upper_bound );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	/**
	 * A Device is one physical thing which is used to interact. 
	 * One device can have multiple handlers.
	 * E.g. one "push button" a.k.a "Knob" device can have handlers for click, push, hold, etc...
	 * Different devices can have different implementations in hardware. E.g. one push button
	 * watches one binary pin. One PowerSensor watches one analog pin. But you can have devices
	 * with multiple pins, too.
	 * All devices are connected and manage queues of handlers.
	 * Handlers are added via the "on" method.
	 */

	class Device {

		friend class Panel;

		private:
			const char *_name;
			Canister<Handler,KNOBS_HANDLER_CANISTER_SIZE> _handlers;

			Device *_slave;

		protected:
			void _activate( knob_value_t newState,
					knob_value_t oldState, knob_time_t count );
			Device( const char *name );
			bool _mute;

		public:
			Device& enslave( Device &slave );
			Device& mute( bool mute );
			virtual void loop() = 0;
			Device& on( Handler &handler );
			const char *name();

			#define ON( WHAT ) \
					inline Device& on ## WHAT( minimal_callback_t cb ) { \
						on( *( new WHAT( cb ) ) ); \
						return *this; \
					}
			#define ONP( WHAT, TYPE ) \
					inline Device& on ## WHAT( minimal_callback_t cb, TYPE val ) { \
						on( *( new WHAT( cb, val ) ) ); \
						return *this; \
					}
			#define ONPP( WHAT, TYPE1, TYPE2 ) \
					inline Device& on ## WHAT( minimal_callback_t cb, TYPE1 val1, TYPE2 val2 ) { \
						on( *( new WHAT( cb, val1, val2 ) ) ); \
						return *this; \
					}

			ON( Always )
			ON( Push )
			ON( Release )
			ON( Toggle )
			ON( Click )
			ON( DoubleClick )
			ON( MultiClick )
			ONP( Hold, knob_time_t )
			ONP( Over, knob_value_t )
			ONP( Under, knob_value_t )
			ONPP( Hysteresis, knob_value_t, knob_value_t )
	};

	/**
	 * BooleanDevice: One device which watches one digital input pin
	 */
	class BooleanDevice : public Device {

		protected:
			const pin_t _pin;

			bool _invert;
			bool _read();

		public:
			BooleanDevice( const char *name, pin_t pin );

			BooleanDevice& pullup( bool on );
			BooleanDevice& invert( bool on );

			pin_t pin();
			
	};

	/**
	 * Knob: An implementation of an boolean device which is debounced.
	 */
	class Knob : public BooleanDevice {

		private:

			static const knob_time_t TIME_DEBOUNCE = 25; //ms

			knob_value_t _value;

			knob_time_t _lastTime;
			knob_time_t _timeUnchanged;

			knob_time_t _timeDebounce;
			knob_time_t _countDebounce;


		public:

			Knob( const char *name, pin_t pin );

			Knob& debounce( knob_time_t time );

			knob_value_t value();

			virtual void loop();
			
	};

	/*
	class Knobber : public Knob {

		private:
			Knob &_other;

		protected:
			void _activate( knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			Knobber( pin_t pin, Knob &other );
			virtual void loop();
	};
	*/

	/**
	 * Small helper class to get your knobs organized.
	 *
	 * If you need to know: The buttons are processed in reversed
	 * order of how they where added. This keeps code size small
	 * and we don't have to lookup/store the last added button(last first) 
	 */
	class Panel {

		private:

			const char *_name;
			Canister<Device, KNOBS_PANEL_CANISTER_SIZE> _devices;

		public:

			Panel( const char *name );

			Panel( const char *name, Device &k1 );
			Panel( const char *name, Device &k1, Device &k2 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
					Device &k5 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
					Device &k5, Device &k6 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
					Device &k5, Device &k6, Device &k7 );
			Panel( const char *name, Device &k1, Device &k2, Device &k3, Device &k4,
					Device &k5, Device &k6, Device &k7, Device &k8 );

			Panel& operator <<( Device &device );

			void loop();

			const char * name();

	};

}

#pragma GCC diagnostic pop

#endif
