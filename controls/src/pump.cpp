#include <Arduino.h>

#include "pump.hpp"

void Pump::init()
{
    // set pump pin as output
    pinMode(pump_pin , OUTPUT);

    // set the pump state to off
    pump_on = false;
    digitalWrite(pump_pin, pump_on);

}

bool Pump::togglePump(bool option)
{
    // check if option is already set
    if(option != pump_on)
    {
        pump_on = option;

        // toggle pump state on/off
        digitalWrite(pump_pin, !pump_on);
    }
    
    // return the new pump state
    return pump_on;
}

bool Pump::is_pump_on()
{
    // return the current pump state
    return pump_on;
}