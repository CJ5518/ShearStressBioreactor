#pragma once

#include <Arduino.h>
#include <stdint.h>

class FlowSensor {
    public:
        // Commands allowed to be read from serial connection
        enum class Command : int {
            sensor_address               = (0x08),
            call_reset                   = (0x0006),
            start_WATER_cont_measurement = (0x3608),
            start_IPA_cont_measurement   = (0x3616),
            stop_cont_measurement        = (0x3FF9)
        };
        
        const int SENSOR_POWERUP = 25; // ms
        const int SENSOR_WARMUP  = 50; // ms
        
        const float SCALE_FACTOR_TEMP = 200.0; // celsius
        
        const char *UNIT_FLOW   = " ml/min";
        const char *UNIT_TEMP_C = " deg C";
        const char *UNIT_TEMP_F = " deg F";
        
        uint16_t aux_value; // flags 
        uint16_t sensor_flow_value;
        uint16_t sensor_temp_value;
        
        int16_t signed_flow_value;
        int16_t signed_temp_value;

        float scaled_flow_value;
        float scaled_temp_value;

        byte aux_crc;
        byte sensor_flow_crc;
        byte sensor_temp_crc;

        void init_sensor();
        void readSensor();
        int set_liquid_type(bool isWater);
        int soft_rst_sensor();
        int start_cont_readings(bool isWater);
        int stop_cont_readings();
};