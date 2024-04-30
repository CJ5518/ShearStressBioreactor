/****************************************************************************************************
 * tcaMultiplexer.cpp
 * Carson Sloan
 * 
 * Defines the I2C_Expander class, which allows communication to either of the two flow sensors that
 * would otherwise have the same I2C address. It may be possible in the future to remove the need for
 * the expander if the addresses of the sensors can be set by this process: 
 * https://sensirion.com/media/documents/15D8B73E/6614FAE3/LQ_AN_SLF3x-I2C-Address-change.pdf.
/*****************************************************************************************************/

#include "tcaMultiplexer.hpp"
#include <Wire.h>

/*
 * Select the device at the provided index.
 */
void I2C_Expander::tcaSelect(uint8_t i) {
    // The total capacity is 8 devices, but our sensors are at 0 and 1
    if (i > 7) {
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
}

/*
 * Select the low sensor if true is provided, otherwise the high sensor is selected.
 */
void I2C_Expander::readSensor(bool lowSensor) {
    if (lowSensor) {
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