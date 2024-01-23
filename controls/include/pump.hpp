#pragma once

#include <ModbusMaster.h>

class Pump {
    public:
        void init(ModbusMaster controller);
        bool togglePump(bool option);
        int setSpeed(int speed);
        bool isPumpOn();
        bool getSpeed();
    private:
        const int PUMP_PIN = 23; // pin connected to the pump relay input
        bool pumpOn; // state of the pump

        ModbusMaster controller;
};