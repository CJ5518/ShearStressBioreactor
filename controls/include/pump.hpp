#pragma once

class Pump {
    public:
        void init();
        bool togglePump(bool option);
        bool isPumpOn();
    private:
        int pumpPin = 23; // pin connected to the pump relay input
        bool pumpOn; // state of the pump
};