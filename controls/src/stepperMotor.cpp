#include <Arduino.h>

#include "stepperMotor.hpp"

void StepperMotor::set_state(StepperMotor::State s)
{
    curr_state = s;

    if(s == State::Off)
    {
        digitalWrite(enaPin, HIGH);
    }
    else
    {
        digitalWrite(enaPin, LOW);
    }
}

StepperMotor::State StepperMotor::get_state()
{
    return curr_state;
}

void StepperMotor::init(unsigned int dir, unsigned int step, unsigned int ena)
{
    dirPin = dir;
    stepPin = step;
    enaPin = ena;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enaPin, OUTPUT);

    set_state(State::Off);
}

int StepperMotor::getNumRotations()
{
    return (m_ticks_open / ticks_per_revolution);
}

void StepperMotor::step()
{
    State temp = get_state();

    if(temp == State::Clockwise)
    {
        // turn motor clockwise to close the valve
        digitalWrite(dirPin, HIGH);
        m_ticks_open--; // tick clockwise = closing the valve's flow
    }
    else if(temp == State::CounterClockWise)
    {
        // turn the motor counterclockwise to open the valve
        digitalWrite(dirPin, LOW);
        m_ticks_open++; // tick counterclockwise = opening the valve's flow
    }
    else 
    {
       // do nothing
    }

    // if the motor state is not off, move the motor
    if(temp != State::Off)
    {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeedFast);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeedFast);
    }
}

void StepperMotor::driveMotor(int numTicks, bool openValve)
{
    int i;

    if(!openValve)
    {
        set_state(State::CounterClockWise);
    }
    else
    {
        set_state(State::Clockwise);
    }
    
    for(i = 0; i < numTicks; i++)
    {
        step();
    }
}