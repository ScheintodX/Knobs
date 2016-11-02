#ifndef LEVER_H
#define LEVER_H

#include <stdint.h>
#include <time.h>
#include "Knob.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

namespace Knobs {

	typedef const uint8_t pin_t;
	typedef uint32_t value_t;

	class AnalogDevice : public Device {

		protected:
			const pin_t _pin;

			value_t _read();

		public:
			AnalogDevice( pin_t pin );

			AnalogDevice& pullup( bool on );
	};

	class Lever : public AnalogDevice {

		private:
			const value_t _min;
			const value_t _max;

			value_t _old;

			value_t _rangeMin;
			value_t _rangeMax;
			value_t _samples;
			value_t _sum;
			value_t _count;

		public:
			Lever( pin_t pin, value_t min, value_t max );

			// Todo: LeverModifier o.ä als Template
			Lever& average( value_t samples );
			Lever& tranpose( value_t _rangeMin, value_t _rangeMax );

			virtual void loop();
	};

	class Change : public Handler {

		public:

			Change( callback_t callback );
			Change( minimal_callback_t callback );

			virtual bool handle( Device &dev,
					value_t newState, value_t oldState, time_t time );
	};

}

#pragma GCC diagnostic pop

#endif
