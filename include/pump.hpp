/****************************************************************************************************
 * pump.hpp
 * Carson Sloan
 * 
 * Declares the Pump class to be used to communicate with the peristaltic pump.
/****************************************************************************************************/

#pragma once

#include <YAAJ_ModbusMaster.h>

class Pump {
    public:
        Pump(YAAJ_ModbusMaster controller);

        bool checkStatus();
        bool setPump(bool option);
        bool togglePump();
        bool setSpeed(uint16_t high, uint16_t low, bool start = false);
        bool setSpeed(int flow, bool force = false);
        bool isPumpOn();
        int32_t getSpeed(bool print);
    private:
        bool pumpOn; // state of the pump

        YAAJ_ModbusMaster controller;

        // Pump speeds in ml/min above which the precision of the pump decreases by a factor of 2
        const int STEP_0 = 8;
        const int STEP_1 = 16;
        const int STEP_2 = 32;
        const int STEP_3 = 64;
        const int STEP_4 = 128;
        const int STEP_5 = 256;

        // Register values corresponding to the above speeds
        const uint16_t STEP_0_CMD = 0x4100;
        const uint16_t STEP_1_CMD = 0x4180;
        const uint16_t STEP_2_CMD = 0x4200;
        const uint16_t STEP_3_CMD = 0x4280;
        const uint16_t STEP_4_CMD = 0x4300;
        const uint16_t STEP_5_CMD = 0x4380;
        const uint16_t STEP_6_CMD = 0x43C8;

        // Number of register value increments to change pump speed by 1 ml/min
        const int RATE_0 = 16;      // 1 / 0.0625 ml/min
        const int RATE_1 = 8;       // 1 / 0.125
        const int RATE_2 = 4;       // 1 / 0.25
        const int RATE_3 = 2;       // 1 / 0.5
        const int RATE_4 = 1;       // 1 / 1
        const double RATE_5 = 0.5;  // 1 / 2
};