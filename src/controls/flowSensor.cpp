#include "flowSensor.hpp"

#include <Wire.h>

/*
 * Reset the flow sensor.
 */
void FlowSensor::init_sensor() {
    int ret;

    do {
        // Soft reset the sensor
        ret = soft_rst_sensor();

        if(ret != 0) {
            delay(500); // wait long enough for chip reset to complete
        }
    } while (ret != 0);
    
    delay(50); // wait long enough for chip reset to complete
}

/*
 * Sets the liquid type for the sensor and returns the response.
 */
int FlowSensor::set_liquid_type(bool isWater) {
    int ret;

    ret = start_cont_readings(isWater);
    if(ret != 0) {
        Serial.println("Error while writing measurement mode command");
    }

    return ret;
}

/*
 * Reads the flow rate data from the current sensor, and sets the global signed_flow_value.
 */
void FlowSensor::readSensor() {
    int ret;

    Wire.requestFrom(0x08, 9);

    if(Wire.available() < 9) {
        Serial.println("Error while reading flow measurement");
    }

    sensor_flow_value = Wire.read() << 8; // read the MSB from the sensor
    sensor_flow_value |= Wire.read(); // read the LSB from the sensor
    sensor_flow_crc = Wire.read();

    sensor_temp_value = Wire.read() << 8; // read the MSB from the sensor
    sensor_temp_value |= Wire.read(); // read the LSB from the sensor
    sensor_temp_crc = Wire.read();

    aux_value = Wire.read() << 8; // read the MSB from the sensor
    aux_value |= Wire.read(); // read the LSB from the sensor
    aux_crc = Wire.read();

    ret = stop_cont_readings();

    if(ret != 0) {
        Serial.println("Error during write measurement mode command");
    }
    else {
        signed_flow_value = (int16_t) sensor_flow_value;
        signed_temp_value = (int16_t) sensor_temp_value;
        scaled_temp_value = ((float) signed_temp_value) / SCALE_FACTOR_TEMP;
    }
}

/*
 * Resets the flow rate sensor.
 */
int FlowSensor::soft_rst_sensor() {
    int ret; 
    
    do {
        Wire.beginTransmission(0x00);
        Wire.write(0x06);
        ret = Wire.endTransmission();

        if (ret != 0) {
            Serial.println("Error while sending soft reset command, retrying...");
            delay(500); // wait long enough for chip reset to complete
        }
    } while(ret != 0);
    
    return ret;
}

/*
 * Starts sensing and returns the response.
 */
int FlowSensor::start_cont_readings(bool isWater) {
    int ret;
    
    Wire.beginTransmission(0x08);
    if(isWater) {
        Wire.write(0x36);
        Wire.write(0x08);
    }
    else {
        Wire.write(0x36);
        Wire.write(0x16);
    }
    
    ret = Wire.endTransmission();
    delay(50);
    return ret;
}

/*
 * Stops the sensor and returns the response.
 */
int FlowSensor::stop_cont_readings() {
    int ret;

    Wire.beginTransmission(0x08);
    Wire.write(0x3F);
    Wire.write(0xF9);
    
    ret = Wire.endTransmission();
    delay(50);
    return ret;
}