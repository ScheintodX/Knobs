#ifndef ACS712_H
#define ACS712_H

#include "Lever.h"

//#warning "ACS712"

namespace Knobs {

	enum ACS_VERSION {
		x05B=0, x20A=1, x30A=2
	};

	class ACS712 : public Lever {

		private:
			const float _scale;
			float _val_avg;
			float _div_avg;
			const float _samples;
			int _count;

		public:

			ACS712( pin_t pin, ACS_VERSION version, knob_value_t max, knob_value_t samples );

			virtual void loop();

	};

}

#endif
