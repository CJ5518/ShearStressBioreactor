#include "highFlowSensor.hpp"

/*
 * Scales the global signed_flow_value for the high flow rate and sets the global scaled_flow_value.
 */
void SLF3S4000B::scaleReadings() {
    scaled_flow_value = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;
}