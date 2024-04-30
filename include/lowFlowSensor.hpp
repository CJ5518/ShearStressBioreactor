/****************************************************************************************************
 * lowFlowSensor.hpp
 * Carson Sloan
 * 
 * Declares the SLF3S1300F (low flow sensor) class, which extends FlowSensor and defines its own
 * scaleReadings and name functions.
/*****************************************************************************************************/

#pragma once

#include "flowSensor.hpp"

/*
 * Defines the scale function for the low flow sensor, which extends the FlowSensor class.
 */
class SLF3S1300F : public FlowSensor {
    public:
        const float SCALE_FACTOR_FLOW = 500.0;

        SLF3S1300F();
        // Override methods
        float scaleReadings(int16_t val);
        const char* name();
};