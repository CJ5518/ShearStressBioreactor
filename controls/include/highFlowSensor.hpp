#pragma once

#include "flowSensor.hpp"

class SLF3S4000B : public FlowSensor
{
    public:
        const float SCALE_FACTOR_FLOW = 32.0;

        void scaleReadings();
};