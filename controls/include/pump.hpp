#pragma once

#include <ModbusMaster.h>

class Pump {
    public:
        void init(ModbusMaster controller);
        bool setPump(bool option);
        bool togglePump();
        int setSpeed(int speed);
        bool isPumpOn();
        bool getSpeed();
    private:
        bool pumpOn; // state of the pump

        ModbusMaster controller;
};