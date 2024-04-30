/****************************************************************************************************
 * stepperMotor.cpp
 * Carson Sloan
 * 
 * Defines the StepperMotor class, which controls the microstep driver pins to move the stepper motor.
/*****************************************************************************************************/

#include "stepperMotor.hpp"
#include <Arduino.h>

/*
 * Sets the ENA input of the stepper driver based on the given state.
 */
void StepperMotor::setState(StepperMotor::State s) {
    state = s;

    if (s == State::Off) {
        digitalWrite(enaPin, HIGH);
    }
    else {
        digitalWrite(enaPin, LOW);
    }
}

/*
 * Returns the current state of the stepper motor.
 */
StepperMotor::State StepperMotor::getState() {
    return state;
}

/*
 * Initialize the pins for controlling the stepper. 
 */
void StepperMotor::init(int dir, int step, int ena) {
    dirPin = dir;
    stepPin = step;
    enaPin = ena;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enaPin, OUTPUT);

    setState(State::Off);
}

/*
 * Returns the number of rotations that have been performed.
 */
int StepperMotor::getNumRotations() {
    return completedTicks / TICKS_PER_REVOLUTION;
}

/*
 * Step one tick in the direction of the current stepper state.
 */
void StepperMotor::step() {
    State temp = getState();

    if (temp == State::Clockwise) {
        // Turns motor clockwise to close the valve
        digitalWrite(dirPin, HIGH);
        completedTicks--; // tick clockwise = closing the valve's flow
    }
    else if (temp == State::CounterClockwise) {
        // Turn the motor counterclockwise to open the valve
        digitalWrite(dirPin, LOW);
        completedTicks++; // tick counterclockwise = opening the valve's flow
    }

    // If the motor state is not off, move the motor
    if (temp != State::Off) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(MOTOR_SPEED_FAST);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(MOTOR_SPEED_FAST);
    }
}

/*
 * Drive the motor in the specified direction for the given number of steps.
 */
void StepperMotor::driveMotor(int numTicks, bool openValve) {
    int i;

    if (!openValve) {
        setState(State::CounterClockwise);
    }
    else {
        setState(State::Clockwise);
    }
    
    for (i = 0; i < numTicks; i++) {
        step();
    }

    Serial.print(getNumRotations());
    Serial.println(" rotations have been done (max 23).");
}