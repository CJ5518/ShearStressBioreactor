#pragma once

#include "flowSensor.hpp"

/*
 * Defines the scale function for the low flow sensor, which extends the FlowSensor class.
 */
class SLF3S1300F : public FlowSensor {
    public:
        const float SCALE_FACTOR_FLOW = 500.0; 

        float scaleReadings(int16_t val);
};