#include "lowFlowSensor.hpp"

/*
 * Scales the global signed_flow_value for the low flow rate and sets the global scaled_flow_value.
 */
void SLF3S1300F::scaleReadings() {
    scaled_flow_value = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;
}