#ifndef LEVER_H
#define LEVER_H

#include "Knob.h"

#include <stdint.h>
#include <time.h>

//#warning "Lever"

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

#ifndef KNOBS_MODIFIER_CANISTER_SIZE
#define KNOBS_MODIFIER_CANISTER_SIZE 5
#endif

namespace Knobs {

	/*****************
	 * AnalogDevice
	 ****************/

	class AnalogDevice : public Device {

		protected:
			const pin_t _pin;

			knob_value_t _read();

		public:
			AnalogDevice( const char *name, pin_t pin );

			AnalogDevice& pullup( bool on );
	};

	class LeverModifier;

	/*****************
	 * Lever
	 *
	 * A Leve is an AnalogDevice with additional information of
	 * the values being sampeld.
	 *
	 * It can have a couple of LeverModifiers which modify
	 * the sampled values before passing them on.
	 ****************/

	class Lever : public AnalogDevice {

		private:
			Canister<LeverModifier,KNOBS_MODIFIER_CANISTER_SIZE> _modifiers;

			knob_value_t _old;
			knob_time_t _lastTime;

		protected:
			virtual bool modify( knob_value_t *val );
			virtual void activate( knob_value_t val );

		public:
			const knob_value_t minValue;
			const knob_value_t maxValue;

		public:
			Lever( const char *name, pin_t pin, knob_value_t min, knob_value_t max );

			Lever& modify( LeverModifier &modifier );

			virtual void loop();
	};

	/*****************
	 * Modifier
	 *
	 * Modify the operation of an Lever
	 ****************/
	class LeverModifier {

		friend class Lever;

		public:
			virtual bool modify( Lever &lever, knob_value_t *val ) = 0;
	};

	/*****************
	 * Transpose
	 *
	 * Changes values to another range
	 ****************/
	class Transpose : public LeverModifier {

		private:
			const knob_value_t _minValue;
			const knob_value_t _range;

		public:
			Transpose( knob_value_t min, knob_value_t max );
			virtual bool modify( Lever &lever, knob_value_t *val );
	};

	/*****************
	 * Average
	 *
	 * Calculate Average over a given amount of samples
	 ****************/
	class Average : public LeverModifier {

		private:
			const knob_value_t _samples;
			knob_value_t _count;
			big_knob_value_t _sum;

		public:
			Average( knob_value_t samples );
			virtual bool modify( Lever &lever, knob_value_t *val );

	};

	/*****************
	 * AverageTime
	 *
	 * Calculate Average over a given time
	 ****************/
	class AverageTime : public LeverModifier {

		private:
			const knob_time_t _time;
			knob_time_t _start;
			big_knob_value_t _sum;
			value_t _count;

		public:
			AverageTime( knob_time_t time );
			virtual bool modify( Lever &lever, knob_value_t *val );

	};

	/*****************
	 * RunningAverage
	 *
	 * Calculate Average continuesly by outweighting the old value
	 * with the new values in a given ratio
	 *
	 *                     samples-1
	 * avg_new = avg_old * --------- + val_new
	 *                      samples
	 *
	 * This method requires a startup period in which no average
	 * value is available.
	 * It uses the first sample as startup value. Then 'samples'
	 * values are collected and averaged according to above method.
	 * Only after '_samples' samples the average is regarded as
	 * "stable" and values are returned.
	 ****************/
	class RunningAverage : public LeverModifier {

		protected:
			knob_float_t _avgValue;
			const knob_value_t _samples;
			knob_value_t _count;

		public:
			RunningAverage( knob_value_t samples );
			virtual bool modify( Lever &lever, knob_value_t *val );

	};

	class Deviation : public AverageTime {

		private:
			//const knob_value_t PRERUN = 1000;
			knob_value_t _minValue;
			knob_value_t _maxValue;

		public:
			Deviation( knob_time_t time );
			virtual bool modify( Lever &lever, knob_value_t *val );
	};

	class RunningDeviation : public RunningAverage {

		private:
			//const knob_value_t PRERUN = 1000;
			knob_value_t _minValue;
			knob_value_t _maxValue;

		public:
			RunningDeviation( knob_value_t samples );
			virtual bool modify( Lever &lever, knob_value_t *val );
	};

}

#pragma GCC diagnostic pop

#endif
