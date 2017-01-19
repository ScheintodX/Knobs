#ifndef KNOB_H
#define KNOB_H

#include <stdint.h>
#include <time.h>
#include <climits>

//#warning "Knob"

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#include "knobs_common.h"


/**
 * Everything is put in namespace Knobs in order to avoid conflicts
 */
namespace Knobs {

	typedef const uint8_t pin_t;
	typedef int32_t knob_value_t;
	typedef int64_t big_knob_value_t;
	typedef int64_t knob_time_t;
	typedef float knob_float_t;

	#define KNOB_VAL_MAX INT_MAX
	#define KNOB_VAL_MIN INT_MIN

	class Device;
	class Handler;
	class Panel;

	typedef void (*minimal_callback_t)( knob_value_t val );
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
		HT_CHANGE=7,
		HT_RISE=8,
		HT_FALL=9,
		HT_OVER=10,
		HT_UNDER=11,
		HT_HYSTERESIS=12
	};


	/**
	 * A handler encapsulates on type of action which is looked for in order to do something.
	 * Examples are: push, click, hold, etc...
	 */
	class Handler {

		friend class Device;

		private:

			Handler *_next;

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

		static const knob_time_t MAX_TIME_CLICK = 250;

		private:
			const knob_time_t _maxTimeClick;
			knob_time_t _timeStart;

		protected:
			Click( HandlerType type, callback_t callback, knob_time_t maxTime );
			Click( HandlerType type, minimal_callback_t callback, knob_time_t maxTime );

		public:
			Click( callback_t callback );
			Click( callback_t callback, knob_time_t maxTime );
			Click( minimal_callback_t callback );
			Click( minimal_callback_t callback, knob_time_t maxTime );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};


	class DoubleClick : public Click {

		private:

			static const knob_time_t MAX_TIME_CLICK = 250;
			static const knob_time_t MAX_TIME_INBETWEEN = 750;

			const knob_time_t _maxTimeInbetween;
			const uint8_t _maxClicks;

			knob_time_t _timeStartSequence;
			knob_value_t _clicks;

		protected:

			virtual bool _callback( Device &dev, knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			DoubleClick( callback_t callback );
			DoubleClick( callback_t callback, knob_value_t maxClicks );
			DoubleClick( callback_t callback, knob_value_t maxClicks, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween );
			DoubleClick( minimal_callback_t callback );
			DoubleClick( minimal_callback_t callback, knob_value_t maxClicks );
			DoubleClick( minimal_callback_t callback, knob_value_t maxClicks, knob_time_t maxTimeClick, knob_time_t maxTimeInbetween );

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
			Hysteresis( callback_t callback, knob_value_t lower_bound, knob_value_t upper_bound );
			Hysteresis( minimal_callback_t callback, knob_value_t lower_bound, knob_value_t upper_bound );

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
			Device *_next;
			Handler * _firstHandler;

		protected:
			void _activate( knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			virtual void loop() = 0;
			Device& on( Handler &handler );
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
			BooleanDevice( pin_t pin );

			BooleanDevice& pullup( bool on );
			BooleanDevice& invert( bool on );
			
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

			Knob( pin_t pin );

			Knob & debounce( knob_time_t time );

			knob_value_t value();

			virtual void loop();

			/*
			Knob & onTrippleClick( callback_t * cb );
			Knob & onNTuppleClick( callback_t * cb, uint8_t clicks );
			*/

	};

	/**
	 * Small helper class to get your knobs organized.
	 *
	 * If you need to know: The buttons are processed in reversed
	 * order of how they where added. This keeps code size small
	 * and we don't have to lookup/store the last added button(last first) 
	 */
	class Panel {

		private:

			Device * _first;

		public:

			Panel();

			Panel( Device &k1 );
			Panel( Device &k1, Device &k2 );
			Panel( Device &k1, Device &k2, Device &k3 );
			//Panel( Device &k1, Device &k2, Device &k3, Device... rest );
			Panel( Device &k1, Device &k2, Device &k3, Device &k4 );
			Panel( Device &k1, Device &k2, Device &k3, Device &k4, Device &k5 );
			Panel( Device &k1, Device &k2, Device &k3, Device &k4, Device &k5,
					Device &k6 );
			Panel( Device &k1, Device &k2, Device &k3, Device &k4, Device &k5,
					Device &k6, Device &k7 );
			Panel( Device &k1, Device &k2, Device &k3, Device &k4, Device &k5,
					Device &k6, Device &k7, Device &k8 );

			Panel& operator <<( Device &device );

			void loop();

	};

}

#pragma GCC diagnostic pop

#endif
