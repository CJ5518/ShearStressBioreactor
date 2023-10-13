#include "highFlowSensor.hpp"

void SLF3S4000B::scaleReadings()
{
    scaled_flow_value = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;

}