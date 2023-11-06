#include <Arduino.h>

#include "stepperMotor.hpp"

/*
 * Sets the ENA input of the stepper driver based on the given state.
 */
void StepperMotor::set_state(StepperMotor::State s) {
    curr_state = s;

    if(s == State::Off) {
        digitalWrite(enaPin, HIGH);
    }
    else {
        digitalWrite(enaPin, LOW);
    }
}

/*
 * Returns the current state of the stepper motor.
 */
StepperMotor::State StepperMotor::get_state() {
    return curr_state;
}

/*
 * Initialize the pins for controlling the stepper. 
 */
void StepperMotor::init(unsigned int dir, unsigned int step, unsigned int ena) {
    dirPin = dir;
    stepPin = step;
    enaPin = ena;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enaPin, OUTPUT);

    set_state(State::Off);
}

/*
 * Returns the number of rotations that have been performed.
 */
int StepperMotor::getNumRotations() {
    return (m_ticks_open / ticks_per_revolution);
}

/*
 * Step one tick in the direction of the current stepper state.
 */
void StepperMotor::step() {
    State temp = get_state();

    if(temp == State::Clockwise) {
        // Turns motor clockwise to close the valve
        digitalWrite(dirPin, HIGH);
        m_ticks_open--; // tick clockwise = closing the valve's flow
    }
    else if(temp == State::CounterClockWise) {
        // Turn the motor counterclockwise to open the valve
        digitalWrite(dirPin, LOW);
        m_ticks_open++; // tick counterclockwise = opening the valve's flow
    }

    // If the motor state is not off, move the motor
    if(temp != State::Off) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeedFast);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeedFast);
    }
}

/*
 * Drive the motor in the specified direction for the given number of steps.
 */
void StepperMotor::driveMotor(int numTicks, bool openValve) {
    int i;

    if(!openValve) {
        set_state(State::CounterClockWise);
    }
    else {
        set_state(State::Clockwise);
    }
    
    for(i = 0; i < numTicks; i++) {
        step();
    }
}