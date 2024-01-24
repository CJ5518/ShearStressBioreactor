#include <Arduino.h>
#include <Wire.h>

#include "tcaMultiplexer.hpp"

/*
 * Select the device at the provided index.
 */
void I2C_Expander::tcaSelect(uint8_t i) {
    if(i > 7) {
        return;
    }

    Wire.beginTransmission(TCA_ADDR);
    Wire.write(1 << i);
    Wire.endTransmission();
}

/*
 * Set the addresses for the sensors and update the devices array for the two we are using.
 */
void I2C_Expander::init(unsigned int lowSensor, unsigned int highSensor) {
    lowSensorAddr = lowSensor;
    highSensorAddr = highSensor;

    for(int i = 0; i < 8; i++) {
        if(i == lowSensor || i == highSensor) {
            activeSensors[i] = 1;
        }
        else {
            activeSensors[i] = 0;
        }
    }
}

/*
 * Select the low sensor if true is provided, otherwise the high sensor is selected.
 */
void I2C_Expander::readSensor(bool lowSensor) {
    if(lowSensor) {
        tcaSelect(lowSensorAddr);
    }
    else {
        tcaSelect(highSensorAddr);
    }
}

/*
 * Returns the address of the low flow rate sensor.
 */
int I2C_Expander::getLowSensorAddr() {
    return lowSensorAddr;
}

/*
 * Returns the address of the high flow rate sensor.
 */
int I2C_Expander::getHighSensorAddr() {
    return highSensorAddr;
}