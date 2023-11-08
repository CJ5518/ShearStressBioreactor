#pragma once

#include "flowSensor.hpp"

/*
 * Defines the scale function for the high flow sensor, which extends the FlowSensor class.
 */
class SLF3S4000B : public FlowSensor {
    public:
        const float SCALE_FACTOR_FLOW = 32.0;

        void scaleReadings();
};