#include "highFlowSensor.hpp"

/*
 * Scales the global signed_flow_value for the high flow rate and sets and returns the global scaled_flow_value.
 */
float SLF3S4000B::scaleReadings(int16_t val) {
    return ((float) val) / SCALE_FACTOR_FLOW;
}