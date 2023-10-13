#pragma once

#include "flowSensor.hpp"

class SLF3S1300F : public FlowSensor
{
    public:
        const float SCALE_FACTOR_FLOW = 500.0; 

        void scaleReadings();
};