/****************************************************************************************************
 * tcaMultiplexer.hpp
 * Carson Sloan
 * 
 * Declares the I2C_Expander class, which allows communication to either of the two flow sensors with
 * the same I2C address.
/*****************************************************************************************************/

#pragma once

#include <stdint.h>

// TODO: determine if this can be eliminated by using RS485 to communicate with the sensors, or by changing the I2C addresses
class I2C_Expander {
    public:
        void tcaSelect(uint8_t i);
        void init(unsigned int lowSensor, unsigned int highSensor);
        void readSensor(bool lowSensor);

        int getLowSensorAddr();
        int getHighSensorAddr();
    private:
        const int TCA_ADDR = 0x70;
        int lowSensorAddr;
        int highSensorAddr;
};