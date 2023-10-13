#pragma once

class Pump
{
    public:
        // init the pump
        void init();
        // turn pump on/off
        bool togglePump(bool option);
        // check the state of the pump
        bool is_pump_on();

    private:
        unsigned int pump_pin = 13; // pin for pump
        bool pump_on = false; // store state of the pump

};