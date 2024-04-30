/****************************************************************************************************
 * flowSensor.hpp
 * Carson Sloan
 * 
 * Declares the FlowSensor class, to be used to communicate with the flow sensors.
/****************************************************************************************************/

#pragma once

#include <Arduino.h>
#include <stdint.h>

class FlowSensor {
    private:
        // Commands allowed to be read from serial connection
        enum class Command : int {
            SensorAddress           = (0x08),
            Reset                   = (0x0006),
            StartWaterMeasurement   = (0x3608),
            StartIPAMeasurement     = (0x3616),
            StopMeasurement         = (0x3FF9)
        };
        
        const int SENSOR_POWERUP = 25; // ms
        const int SENSOR_WARMUP  = 50; // ms
        
        const float SCALE_FACTOR_TEMP = 200.0; // celsius
        
        const char *UNIT_FLOW   = " ml/min";
        const char *UNIT_TEMP_C = " deg C";
        const char *UNIT_TEMP_F = " deg F";
        
        uint16_t auxValue; // flags 
        uint16_t sensorFlowValue;
        uint16_t sensorTempValue;
        
        int16_t signedFlowValue;
        int16_t signedTempValue;

        float scaledFlowValue;
        float scaledTempValue;

        byte auxCRC;
        byte sensorFlowCRC;
        byte sensorTempCRC;

    public:
        void initSensor();
        int16_t readSensor();
        int setLiquid(bool isWater);
        int resetSensor();
        int startReading(bool isWater);
        int stopReading();

        virtual const char* name() = 0;
        virtual float scaleReadings(int16_t val) = 0;
};