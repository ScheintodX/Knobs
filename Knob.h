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


// Everything is put in namespace Knobs in order to avoid conflicts
namespace Knobs {

	#define KNOB_VAL_MAX INT_MAX
	#define KNOB_VAL_MIN INT_MIN

	class Device;
	class Handler;
	class Panel;

	typedef bool (*minimal_callback_t)( knob_value_t val );
	typedef bool (*callback_t)( Device &dev, Handler &handler,
			knob_value_t newState, knob_value_t oldState, knob_time_t count );

	class Callable {
		public:
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) = 0;
	};
	class SimpleCallable : public Callable {
		public:
			virtual bool call( knob_value_t val ) = 0;
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count )
					{ return call( newState ); }
	};
	class FunctionCallback : public Callable {
		private:
			const callback_t _cb;
		public:
			FunctionCallback( callback_t cb ) : _cb( cb ) {}
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) {
				 return _cb( dev, handler, newState, oldState, count );
			}
	};
	class MinimalFunctionCallback : public Callable {
		private:
			const minimal_callback_t _cb;
		public:
			MinimalFunctionCallback( minimal_callback_t cb ) : _cb( cb ) {}
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) {
				 return _cb( newState );
			}
	};

	template <class T>
	using callback_method_t = bool (T::*)( Device &dev, Handler &handler,
			knob_value_t newState, knob_value_t oldState, knob_time_t count );

	template <class T>
	class MethodCallback : public Callable {
		private:
			T* _obj;
			callback_method_t<T> _cb;
		public:
			MethodCallback( void *obj, callback_method_t<T> cb ) : _obj( obj ), _cb( cb ) {}
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) {
				 return _cb( _obj, dev, handler, newState, oldState, count );
			}

	};

	template <class T>
	using minimal_callback_method_t = bool (T::*)( knob_value_t );

	template <class T>
	class MinimalMethodCallback : public Callable {
		private:
			T* _obj;
			minimal_callback_method_t<T> _cb;
		public:
			MinimalMethodCallback( T *obj, minimal_callback_method_t<T> cb ) : _obj( obj ), _cb( cb ) {}
			virtual bool call( Device &dev, Handler &handler,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) {
				 return (_obj->*_cb)( newState );
			}
	};

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
		HT_HYSTERESIS=13,
		HT_DELAYED_CHOISE=14,
		HT_TRANSPORT=99
	};

	 // A handler encapsulates on type of action which is looked for in order to do something.
	 // Examples are: push, click, hold, etc...
	 // Handlers have one callback which is called then the Handler thinks it should be.
	class Handler {

		friend class Device;

		protected:

			Callable *_ca;

			virtual bool _callback( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:

			const HandlerType type;

			Handler( HandlerType type, Callable *callback );

			// called periodically with Knob's state
			// Override to implement Handlers functionallity
			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count ) = 0;

	};

	// Callback every loop
	class Always : public Handler {

		public:

			Always( Callable *callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	// Calls back when state changes off -> on
	class Push : public Handler {

		public:

			Push( Callable *callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};
	typedef Push Activate;

	// Calls back when state changes on -> off
	class Release : public Handler {

		public:

			Release( Callable *callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};
	typedef Release Deactivate;

	// Calls back when state changes in any way
	class Toggle : public Handler {

		public:
			Toggle( Callable *callback );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// A special handler for implementing communication
	// Calls back periodically when state is on and a single time when state changes on->off
	class Transport : public Handler {

		private:
			knob_time_t _lastTime;
			const knob_time_t _periode;

		public:
			Transport( Callable *callback, knob_time_t periode );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// Calls back when state changes off -> on -> off
	// while the max time of on can be set
	class Click : public Handler {

		static const knob_time_t MAX_TIME_CLICK = 500;

		private:
			const knob_time_t _maxTimeClick;
			knob_time_t _timeStart;

		protected:
			Click( HandlerType type, Callable *callback,
					knob_time_t maxTimeClick = MAX_TIME_CLICK );

		public:
			Click( Callable *callback, knob_time_t maxTimeClick = MAX_TIME_CLICK );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// Calls back on double click
	class DoubleClick : public Click {

		private:

			static const knob_time_t MAX_TIME_CLICK = 0;
			static const knob_time_t MAX_TIME_INBETWEEN = 750;

			const knob_time_t _maxTimeInbetween;
			const uint8_t _maxClicks;

			knob_time_t _timeLastClick;
			knob_value_t _clicks;

		protected:

			virtual bool _callback( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t count );

		public:
			DoubleClick( Callable *callable,
					knob_value_t maxClicks = 2,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	// Calls back every click after the first
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
			MultiClick( Callable *callable,
					knob_time_t maxTimeClick = MAX_TIME_CLICK,
					knob_time_t maxTimeInbetween = MAX_TIME_INBETWEEN
			);

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	// Calls back periodically while state 'on'
	class Hold : public Handler {

		private:
			const knob_time_t _timeHold;

			bool _continues;
			bool _hasSent;

		public:

			Hold( Callable *callable, knob_time_t time );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

			Hold& continues( bool on );

	};

	// Calls back while value is bigger than configured one
	// Analog: yes
	class Over : public Handler {

		private:
			const knob_value_t _val;

		public:

			Over( Callable *callable, knob_value_t val );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// Calls back while value is lesser than configured one
	// Analog: yes
	class Under : public Handler {

		private:
			const knob_value_t _val;

		public:

			Under( Callable *callable, knob_value_t val );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// Calls back while state is bigger then a configured value
	// and stops when state is lower then another value
	class Hysteresis : public Handler {

		private:
			const knob_value_t _upper_bound;
			const knob_value_t _lower_bound;

		public:

			Hysteresis( Callable *callable,
					knob_value_t lower_bound, knob_value_t upper_bound );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );

	};

	class SlowAveragingHysteresis : public Handler {

		private:
			static const unsigned long _DELAY = 1000;

			const float _upper_bound;
			const float _lower_bound;
			const int _averaging;

			knob_time_t _lastTime;
			float _value;

		public:

			SlowAveragingHysteresis( Callable *callable,
					float lower_bound, float upper_bound, int averaging );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
	};

	// A Device is one physical thing which is used to interact.
	// One device can have multiple handlers.
	// E.g. one "push button" a.k.a "Knob" device can have handlers for click, push, hold, etc...
	// Different devices can have different implementations in hardware. E.g. one push button
	// watches one binary pin. One PowerSensor watches one analog pin. But you can have devices
	// with multiple pins, too.
	// All devices are connected and manage queues of handlers.
	// Handlers are added via the "on" method.
	class Device {

		friend class Panel;

		private:
			const char *_name;
			Canister<Handler,KNOBS_HANDLER_CANISTER_SIZE> _handlers;

			bool _mute;

			Device *_slave;

		protected:
			void _activate( knob_value_t newState,
					knob_value_t oldState, knob_time_t count );

			Device( const char *name );

		public:
			// remote controll other Device
			Device& enslave( Device &slave );

			// pause operation
			Device& mute( bool mute );
			bool mute();

			// must be called periodically. At best < every 20ms
			virtual void loop() = 0;

			// add a handler
			Device& on( Handler *handler );
			Device& on( Handler &handler );

			// return name
			const char *name();

			#define ON( WHAT ) \
					inline Device& on ## WHAT( Callable &cb ) { \
						on( new WHAT( &cb ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( minimal_callback_t cb ) { \
						on( new WHAT( new MinimalFunctionCallback( cb ) ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( callback_t cb ) { \
						on( new WHAT( new FunctionCallback( cb ) ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, minimal_callback_method_t<T> cb ) { \
						on( new WHAT( new MinimalMethodCallback<T>( obj, cb ) ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, callback_method_t<T> cb ) { \
						on( new WHAT( new MethodCallback<T>( obj, cb ) ) ); \
						return *this; \
					}
			#define ONP( WHAT, TYPE ) \
					inline Device& on ## WHAT( Callable &cb, TYPE val ) { \
						on( new WHAT( &cb, val ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( minimal_callback_t cb, TYPE val ) { \
						on( new WHAT( new MinimalFunctionCallback( cb ), val ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( callback_t cb, TYPE val ) { \
						on( new WHAT( new FunctionCallback( cb ), val ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, minimal_callback_method_t<T> cb, TYPE val ) { \
						on( new WHAT( new MinimalMethodCallback<T>( obj, cb ), val ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, callback_method_t<T> cb, TYPE val ) { \
						on( new WHAT( new MethodCallback<T>( obj, cb ), val ) ); \
						return *this; \
					}

			#define ONPP( WHAT, TYPE1, TYPE2 ) \
					inline Device& on ## WHAT( Callable &cb, TYPE1 val1, TYPE2 val2 ) { \
						on( new WHAT( &cb, val1, val2 ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( minimal_callback_t cb, TYPE1 val1, TYPE2 val2 ) { \
						on( new WHAT( new MinimalFunctionCallback( cb ), val1, val2 ) ); \
						return *this; \
					} \
					inline Device& on ## WHAT( callback_t cb, TYPE1 val1, TYPE2 val2 ) { \
						on( new WHAT( new FunctionCallback( cb ), val1, val2 ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, minimal_callback_method_t<T> cb, TYPE1 val1, TYPE2 val2 ) { \
						on( new WHAT( new MinimalMethodCallback<T>( obj, cb ), val1, val2 ) ); \
						return *this; \
					} \
					template<class T> \
					inline Device& on ## WHAT( T *obj, callback_method_t<T> cb, TYPE1 val1, TYPE2 val2 ) { \
						on( new WHAT( new MethodCallback<T>( obj, cb ), val1, val2 ) ); \
						return *this; \
					}

			// add handlers. see there for what they do.
			// note that these methods must use 'new' to create the handlers
			// so keep an eye on the heap
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
			ONP( Transport, knob_time_t )
	};

	// BooleanDevice: One device which watches one digital input pin
	class BooleanDevice : public Device {

		private:
			const pin_t _pin;

		protected:

			bool _invert;

			bool _read();

		public:
			BooleanDevice( const char *name, pin_t pin );

			// activate pullup (if hardware supports it)
			BooleanDevice& pullup( bool on );

			// invert logical pin state
			BooleanDevice& invert( bool on );

			pin_t pin();
	};

	// Knob: An implementation of an boolean device which is debounced.
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

	// Small helper class to get your knobs organized.
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

			// add another Device
			Panel& operator <<( Device &device );

			// call periodicalle. At best faster than 20ms
			void loop();

			// return name
			const char * name();

	};
}

#pragma GCC diagnostic pop

#endif
