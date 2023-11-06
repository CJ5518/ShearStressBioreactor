#pragma once

class Pump {
    public:
        void init();
        bool togglePump(bool option);
        bool is_pump_on();
    private:
        unsigned int pump_pin = 23; // pin connected to the pump relay input
        bool pump_on; // state of the pump
};