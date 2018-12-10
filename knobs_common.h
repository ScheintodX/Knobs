#ifndef KNOBS_COMMON_H
#define KNOBS_COMMON_H

#include <stdint.h>

namespace Knobs {

	typedef uint8_t pin_t;
	typedef uint32_t value_t;
	typedef int32_t knob_value_t;
	typedef int64_t big_knob_value_t;
	typedef int64_t knob_time_t;
	typedef float knob_float_t;

	typedef void (*debugger_t)( const char *msg ); // + float value?

}

#endif
