#include "lowFlowSensor.hpp"

void SLF3S1300F::scaleReadings()
{
    scaled_flow_value = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;
}