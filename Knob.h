#ifndef KNOB_H
#define KNOB_H

#include <stdint.h>
#include <time.h>

namespace Knobs {


	typedef const uint8_t pin_t;
	typedef uint32_t value_t;

	class Device;
	class Handler;
	class Panel;

	typedef bool (*callback_t)( Device &dev, value_t newState, value_t oldState, time_t count );


	class Handler {

		friend Device;

		private:

			Handler *_next;

		protected:

			const callback_t _cb;

		public:

			Handler( callback_t callback );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t count ) = 0;

	};


	class Push : public Handler {

		public:

			Push( callback_t callback );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t time );

	};

	class Release : public Handler {

		public:

			Release( callback_t callback );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t time );

	};

	class Click : public Handler {

		private:
			const time_t _maxTimeClick;
			time_t _timeStart;

		public:
			Click( callback_t callback, time_t maxTime );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t time );
	};


	class Hold : public Handler {

		private:
			const time_t _timeHold;

			bool _continues;
			bool _hasSent;

		public:

			Hold( callback_t callback, time_t time );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t time );

			Hold& continues( bool on );

	};

	/**
	 * Push, Hold, Release, Click, DoubleClick
	 */


	/*
	class Click : Handler {

		public:

			Click( callback_t callback );

			virtual bool handle( Device &dev, value_t newState, value_t oldState, time_t time );

	};
	*/

	/*
	class Push : Handler { };
	class Release : Handler { };

	class Hold : Handler {
	
		private:
			static const time_t TIME_HOLD = 500;

			time_t _timeHold;

		public:
			Hold& : continues( bool continues );
	};

	class Click : Handler { };

	class Change : Handler { };
	*/



	class Device {

		friend Panel;

		private:
			Device *_next;
			Handler * _firstHandler;

		protected:
			void _activate( bool newState, bool oldState, time_t count );

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

	class Lever {

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

			Panel& operator <<( Device &device );

			void loop();

	};

};
#endif

