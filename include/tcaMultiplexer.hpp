#pragma once

#include <stdint.h>

class I2C_Expander {
    public:
        void tcaSelect(uint8_t i);
        void init(unsigned int lowSensor, unsigned int highSensor);
        void readSensor(bool lowSensor);

        int getLowSensorAddr();
        int getHighSensorAddr();
    private:
        const int TCA_ADDR = 0x70;
        int activeSensors[8];
        int lowSensorAddr;
        int highSensorAddr;
};