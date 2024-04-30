/****************************************************************************************************
 * highFlowSensor.cpp
 * Carson Sloan
 * 
 * Defines the SLF3S4000B class for functions specific to the high flow sensor.
/*****************************************************************************************************/

#include "highFlowSensor.hpp"

/*
* Constructor for a new high flow sensor object that initially sends the reset command.
*/
SLF3S4000B::SLF3S4000B() {
    initSensor();
}

/*
 * Scales the global signed_flow_value for the high flow rate and sets and returns the global scaled_flow_value.
 */
float SLF3S4000B::scaleReadings(int16_t val) {
    return ((float) val) / SCALE_FACTOR_FLOW;
}

/*
 * Returns the name of this sensor.
 */
const char* SLF3S4000B::name() {
    return "SLF3S4000B high flow sensor";
}