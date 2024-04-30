/****************************************************************************************************
 * flowManager.hpp
 * Carson Sloan
 * 
 * Declares the FlowManager class and member functions.
/****************************************************************************************************/

#pragma once

#include "pump.hpp"
#include "lowFlowSensor.hpp"
#include "highFlowSensor.hpp"
#include "stepperMotor.hpp"
#include "tcaMultiplexer.hpp"
#include <PIDController.h>

class FlowManager {
    private:
        const int LOW_ADDRESS = 0;
        const int HIGH_ADDRESS = 1;

        // Flow rate limits
        const float MIN_LOW_FLOW_RATE = 0.0;
        const float MAX_LOW_FLOW_RATE = 27.09;
        const float MIN_HIGH_FLOW_RATE = 180.0;
        const float MAX_HIGH_FLOW_RATE = 361.17;

        // Stepper motor limits
        const int STEPS_PER_TURN = 4500; // steps per full rotation of the valve
        const int MAX_TURNS = 21.5; // number of turns before the shorter valve is fully open
        const int MAX_SPEED = 10000; // fastest steps/second to safely drive motors
        const int MIN_SPEED = 1; // no negative values should be needed, since the direction is set beforehand

        // Output variable for the PID controller
        float ticksToDrive;
        PIDController pid;
        Pump* pump;

        bool lastSys; // whether or not the last flow rate requested was for the low flow rate system

        I2C_Expander tca;
        SLF3S1300F lowFS;
        SLF3S4000B highFS;

        StepperMotor lowMotor;
        StepperMotor highMotor;

        void init();
    public:
        int stepsTaken = 0;

        FlowManager(Pump* p);
        void setFlow(float tgtFlowRate);
        void closeFlow(bool lowFlow);
        float takeAvgNumReadings(bool lowFlow, int numReadings, bool print = false);
};

