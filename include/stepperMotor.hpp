/****************************************************************************************************
 * stepperMotor.hpp
 * Carson Sloan
 * 
 * Declares the StepperMotor class, each object of which controls a microstep driver.
/*****************************************************************************************************/

#pragma once

class StepperMotor {
    public:
        // Possible stepper states
        enum class State {Off, Clockwise, CounterClockwise};
        
        void init(int dir, int step, int ena);
        
        State getState();
        void setState(State s);
        void step();
        void driveMotor(int numTicks, bool openValve);
        int getNumRotations();

    private:
        const int TICKS_PER_REVOLUTION = 6400; // TODO: determine whether this value is correct

        // Number of us 1 tick takes at each DIP precision setting
        const int MOTOR_SPEED_FAST = 100;
        /*const int motorSpeedMed = 500;
        const int motorSpeedSlow = 1000;
        const int motorSpeedVerySlow = 10000;*/

        int dirPin;
        int stepPin;
        int enaPin;

        State state;

        int completedTicks; // number of ticks in current rotation
};