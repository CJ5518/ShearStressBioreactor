#pragma once

#include "lowFlowSensor.hpp"
#include "highFlowSensor.hpp"
#include "stepperMotor.hpp"
#include "tcaMultiplexer.hpp"
#include <PIDController.h>

class FlowManager {
    private:
        const float min_lowFlowRate = 0.0;
        const float max_lowFlowRate = 27.09;
        const float min_highFlowRate = 180.0;
        const float max_highFlowRate = 361.17;

        // Output variable for the PID controller
        float ticks_toDrive;
        PIDController pid;

        bool lastSys; // whether or not the last flow rate requested was for the low flow rate system

        const int slice_time = 10; // time between adjustments in ms
        const int steps_per_turn = 4500; // steps per full rotation of the valve
        const int turns = 22.5; // number of turns before the shorter valve is fully open
        const int max_speed = 10000; // fastest steps/second to safely drive motors
        const int min_speed = 1; // no negative values should be needed, since the direction is set beforehand

        I2C_Expander tca;
        SLF3S1300F lowFS;
        SLF3S4000B highFS;

        StepperMotor low_Motor;
        StepperMotor high_Motor;
    public:
        void init();
        void setFlow(float tgt_flowRate, bool isWater);
        void closeFlow(bool lowFlow);
        float takeAvgNumReadings(bool lowFlow, int numReadings);
        int stepsTaken = 0;
};

