#include <Arduino.h>
#include <Wire.h>

#include "TCA_multiplexer.hpp"

void I2C_Expander::tcaselect(uint8_t i)
{
    if(i > 7)
    {
        return;
    }
    Wire.beginTransmission(TCA_ADDR);
    Wire.write(1 << i);
    Wire.endTransmission();
}

void I2C_Expander::init(unsigned int lowSensor, unsigned int highSensor)
{
    lowSensorAddr = lowSensor;
    highSensorAddr = highSensor;

    for(int i = 0; i < 8; i++)
    {
        if(i == lowSensor || i == highSensor)
        {
            active_sensors[i] = 1;
        }
        else
        {
            active_sensors[i] = 0;
        }
    }
}

void I2C_Expander::readSensor(bool lowSensor)
{
    if(lowSensor)
    {
        tcaselect(lowSensorAddr);
    }
    else
    {
        tcaselect(highSensorAddr);
    }
}

int I2C_Expander::getLowSensorAddr()
{
    return lowSensorAddr;
}

int I2C_Expander::getHighSensorAddr()
{
    return highSensorAddr;
}