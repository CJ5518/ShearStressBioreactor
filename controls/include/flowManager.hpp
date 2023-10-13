#pragma once

#include "lowFlowSensor.hpp"
#include "highFlowSensor.hpp"
#include "stepperMotor.hpp"
#include "TCA_multiplexer.hpp"

class flowManager
{
    private:
        const float min_lowFlowRate = 0.0;
        const float max_lowFlowRate = 27.09;
        const float min_highFlowRate = 180.0;
        const float max_highFlowRate = 361.17;

        
        
        I2C_Expander tca;

        SLF3S1300F low_FS;
        SLF3S4000B high_FS;

        StepperMotor low_Motor;
        StepperMotor high_Motor;

    public:
        void init();
        void readSensor();
        void test_flow(float tgt_flowRate, bool isWater);
        void close_flow(bool lowFlow);
        void takeAvgNumReadings(bool lowFlow, int numReadings);
        int stepsTaken = 0;
        float flowAvg;
};

