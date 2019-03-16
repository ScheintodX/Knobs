#ifndef ROTARY_H
#define ROTARY_H

#include "Arduino.h"
#include "Knob.h"

namespace Knobs {

    const int _NUM_ROT=8;

    typedef uint8_t pin_t;

    class Rotary : public Device {

        private:

            static Rotary *_POOL[ _NUM_ROT ];
            static uint8_t _IDX;

            #define _CB( a ) \
                    static void _##a##_1_c(){ Rotary::_POOL[ a ]->_c1(); }; \
                    static void _##a##_2_c(){ Rotary::_POOL[ a ]->_c2(); }
            _CB( 0 );
            _CB( 1 );
            _CB( 2 );
            _CB( 3 );
            _CB( 4 );
            _CB( 5 );
            _CB( 6 );
            _CB( 7 );

        private:
            const pin_t _p1, _p2;
            const uint8_t _idx;
            const uint8_t _div;

            int _v1, _v2;
            int32_t _value, _lastValue;
            bool _invert;


        public:
            Rotary( const char *name, const pin_t p1, const pin_t p2, uint8_t div );

            Rotary( const char *name, pin_t p1, pin_t p2 ) : Rotary( name, p1, p2, 1 ){};

            Rotary& pullup( bool pullup );
            Rotary& invert( bool invert );

            virtual void loop();

        private:

            Rotary& _start();

        public:
            int32_t value();

        private:

            int32_t _read(){ return _value / _div; }

            void _c1();
            void _r1();
            void _f1();

            void _c2();
            void _r2();
            void _f2();
    };

    /**
     * Handle Changes of value but wait 'delay' for no further change to appear
     */
    class DelayedChoise : public Handler {

        private:
            const knob_time_t _delay;

            knob_time_t _lastTime;
            knob_value_t _sum;

		public:
            DelayedChoise( Callable *callable, knob_time_t delay );

			virtual bool handle( Device &dev,
					knob_value_t newState, knob_value_t oldState, knob_time_t time );
    };

}

#endif
