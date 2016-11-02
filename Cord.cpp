#include "Cord.h"

#include <Arduino.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wpedantic"
#pragma GCC diagnostic error "-Wreturn-type"

using namespace Knobs;

void y() {

}

void x() {

	SmallFitting f( y );

	Click click( SmallFitting( y ) );

}


#pragma GCC diagnostic pop
