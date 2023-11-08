#pragma once

class StepperMotor {
    public:
        // Possible stepper states
        enum class State {Off, Clockwise, CounterClockWise};
        
        void init(unsigned int dir, unsigned int step, unsigned int ena);
        
        State get_state();
        void set_state(State s);
        void step();
        void driveMotor(int numTicks, bool openValve);
        int getNumRotations();
        
        float percent_through_revolution();
        int min_us_between_steps();
        int max_us_between_steps();

    private:
        const int ticks_per_revolution = 6400;

        const int motorSpeedFast = 100;
        const int motorSpeedMed = 500;
        const int motorSpeedSlow = 1000;
        const int motorSpeedVerySlow = 10000;

        unsigned int dirPin;
        unsigned int stepPin;
        unsigned int enaPin;

        State curr_state;

        int m_ticks_open; // number of ticks in current rotation
};