/****************************************************************************************************
 * lowFlowSensor.cpp
 * Carson Sloan
 * 
 * Defines the SLF3S1300F class for functions specific to the low flow sensor.
/*****************************************************************************************************/

#include "lowFlowSensor.hpp"

/*
* Constructor for a new low flow sensor object that initially  sends the reset command.
*/
SLF3S1300F::SLF3S1300F() {
    initSensor();
}

/*
 * Scales the global signed_flow_value for the low flow rate and sets the global scaled_flow_value.
 */
float SLF3S1300F::scaleReadings(int16_t val) {
    return ((float) val) / SCALE_FACTOR_FLOW;
}

/*
 * Returns the name of this sensor.
 */
const char* SLF3S1300F::name() {
    return "SLF3S1300F low flow sensor";
}