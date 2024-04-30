/****************************************************************************************************
 * flowSensor.cpp
 * Carson Sloan
 * 
 * Defines the FlowSensor class, to be used to communicate with the flow sensors.
/****************************************************************************************************/

#include "flowSensor.hpp"
#include <Wire.h>

/*
 * Resets the flow sensor.
 */
void FlowSensor::initSensor() {
    int ret;

    // Soft reset the sensor
    ret = resetSensor();

    if (ret != 0) {
        Serial.printf("The %s is disconnected!\n", name());
    }
}

/*
 * Sets the liquid type for the sensor and returns the response.
 */
int FlowSensor::setLiquid(bool isWater) {
    int ret;

    ret = startReading(isWater);
    if (ret != 0) {
        Serial.printf("Error %d while sending measurement mode command to the %s.\n", ret, name());
    }

    return ret;
}

/*
 * Reads the flow rate data from the current sensor, sets the global flow values, and returns the signed flow value.
 */
int16_t FlowSensor::readSensor() {
    int ret;

    Wire.requestFrom(0x08, 9);

    if (Wire.available() < 9) {
        Serial.printf("Error while reading flow measurement from the %s.\n", name());
    }

    sensorFlowValue = Wire.read() << 8; // read the MSB from the sensor
    sensorFlowValue |= Wire.read(); // read the LSB from the sensor
    sensorFlowCRC = Wire.read();
    sensorTempValue = Wire.read() << 8; // read the MSB from the sensor
    sensorTempValue |= Wire.read(); // read the LSB from the sensor
    sensorTempCRC = Wire.read();

    auxValue = Wire.read() << 8; // read the MSB from the sensor
    auxValue |= Wire.read(); // read the LSB from the sensor
    auxCRC = Wire.read();

    ret = stopReading();

    if (ret != 0) {
        Serial.printf("Error while sending stop reading command to the %s.\n", name());
    }
    else {
        signedFlowValue = (int16_t) sensorFlowValue;
        signedTempValue = (int16_t) sensorTempValue;
        scaledTempValue = ((float) signedTempValue) / SCALE_FACTOR_TEMP; // TODO: determine if this is useful
    }

    return signedFlowValue;
}

/*
 * Resets the flow rate sensor.
 */
int FlowSensor::resetSensor() {
    int ret = 1;
    int failures = 0;
    
    while (ret != 0) {
        Wire.beginTransmission(0x00);
        Wire.write(0x06);
        ret = Wire.endTransmission();

        if (ret != 0) {
            // Stop trying after about 5 seconds of no response
            if (failures > 50) {
                return ret;
            }

            Serial.printf("Error %d while sending reset command to the %s.\n", ret, name());
            failures++;
            delay(100); // wait long enough for chip reset to complete
        }
    }
    
    return ret;
}

/*
 * Starts sensing and returns the response.
 */
int FlowSensor::startReading(bool isWater) {    
    Wire.beginTransmission(0x08);
    
    // Tell the sensor whether this is water or IPA
    if (isWater) {
        Wire.write(0x36);
        Wire.write(0x08);
    }
    else {
        Wire.write(0x36);
        Wire.write(0x16);
    }
    
    int ret = Wire.endTransmission();

    return ret;
}

/*
 * Stops the sensor and returns the response.
 */
int FlowSensor::stopReading() {
    Wire.beginTransmission(0x08);
    Wire.write(0x3F);
    Wire.write(0xF9);
    
    int ret = Wire.endTransmission();

    return ret;
}