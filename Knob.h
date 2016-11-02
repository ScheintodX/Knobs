#ifndef KNOB_H
#define KNOB_H

#include <stdint.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#include "knobs_common.h"

namespace Knobs {

	typedef const uint8_t pin_t;
	typedef uint32_t value_t;

	class Device;
	class Handler;
	class Panel;

	typedef void (*minimal_callback_t)();
	typedef bool (*callback_t)( Device &dev, Handler &handler, value_t newState, value_t oldState, time_t count );

	enum HandlerType {
		HT_PUSH=0, HT_ACTIVATE=0,
		HT_RELEASE=1, HT_DEACTIVATE=1,
		HT_HOLD=2,
		HT_CLICK=3,
		HT_TOGGLE=4,
		HT_DOUBLECLICK=5,
		HT_CHANGE=6,
		HT_RISE=7,
		HT_FALL=8
	};

	class Fitting {

		public:
			virtual bool call( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count ) = 0;
	};

	class SmallFitting : public Fitting {
		private:
			minimal_callback_t _callback;
		public:
			SmallFitting( minimal_callback_t callback );
			virtual bool call( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count );
	};

	class MediumFitting : public Fitting {
		private:
			bool (*callback)();
		public:
			virtual bool call( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count );
	};

	class LargeFitting : public Fitting {
		private:
			bool (*callback)();
		public:
			virtual bool call( Device &dev, Handler &h, value_t newValue, value_t oldValue, time_t count );
	};


	class Handler {

		friend class Device;

		private:

			Handler *_next;

		protected:

			const callback_t _cb;
			const minimal_callback_t _cbm;

			virtual bool _callback( Device &dev, value_t newState, value_t oldState, time_t count );

		public:

			const HandlerType type;

			Handler( HandlerType type, minimal_callback_t callback );
			Handler( HandlerType type, callback_t callback );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t count ) = 0;

	};


	class Push : public Handler {

		public:

			Push( callback_t callback );
			Push( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );

	};
	typedef Push Activate;

	class Release : public Handler {

		public:

			Release( callback_t callback );
			Release( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );

	};
	typedef Release Deactivate;

	class Toggle : public Handler {

		public:
			Toggle( callback_t callback );
			Toggle( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );
	};

	class Click : public Handler {

		static const time_t MAX_TIME_CLICK = 250;

		private:
			const time_t _maxTimeClick;
			time_t _timeStart;

		protected:
			Click( HandlerType type, callback_t callback, time_t maxTime );
			Click( HandlerType type, minimal_callback_t callback, time_t maxTime );

		public:
			Click( callback_t callback );
			Click( callback_t callback, time_t maxTime );
			Click( minimal_callback_t callback );
			Click( minimal_callback_t callback, time_t maxTime );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );
	};


	class DoubleClick : public Click {

		private:

			static const time_t MAX_TIME_CLICK = 250;
			static const time_t MAX_TIME_INBETWEEN = 750;

			const time_t _maxTimeInbetween;
			const uint8_t _maxClicks;

			time_t _timeStartSequence;
			value_t _clicks;

		protected:

			virtual bool _callback( Device &dev, value_t newState, value_t oldState, time_t count );

		public:
			DoubleClick( callback_t callback );
			DoubleClick( callback_t callback, value_t maxClicks );
			DoubleClick( callback_t callback, value_t maxClicks, time_t maxTimeClick, time_t maxTimeInbetween );
			DoubleClick( minimal_callback_t callback );
			DoubleClick( minimal_callback_t callback, value_t maxClicks );
			DoubleClick( minimal_callback_t callback, value_t maxClicks, time_t maxTimeClick, time_t maxTimeInbetween );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );

	};

	class Hold : public Handler {

		private:
			const time_t _timeHold;

			bool _continues;
			bool _hasSent;

		public:

			Hold( callback_t callback, time_t time );
			Hold( minimal_callback_t callback, time_t time );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );

			Hold& continues( bool on );

	};


	class Device {

		friend class Panel;

		private:
			Device *_next;
			Handler * _firstHandler;

		protected:
			void _activate( value_t newState, value_t oldState, time_t count );

		public:
			virtual void loop() = 0;
			Device& on( Handler &handler );
	};

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

	class Knob : public BooleanDevice {

		private:

			static const time_t TIME_DEBOUNCE = 25; //ms

			value_t _value;

			time_t _lastTime;
			time_t _timeUnchanged;

			time_t _timeDebounce;
			time_t _countDebounce;

		public:

			Knob( pin_t pin );

			Knob & debounce( time_t time );

			value_t value();

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
