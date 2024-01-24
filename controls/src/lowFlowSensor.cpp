#include "lowFlowSensor.hpp"

/*
 * Scales the global signed_flow_value for the low flow rate and sets the global scaled_flow_value.
 */
float SLF3S1300F::scaleReadings(int16_t val) {
    return ((float) val) / SCALE_FACTOR_FLOW;
}