#include <Arduino.h>

#include "pump.hpp"

/*
 * Initialize the pump pin and turn the pump off.
 */
void Pump::init() {
    // Set the pump pin as output
    pinMode(pumpPin, OUTPUT);

    // Set the pump state to on since it has power by default for some reason
    pumpOn = true;
    togglePump(false); // turn the pump off
}

/*
 * Turns the pump on if true is provided, off if false. The state of the pump is returned afterward.
 */
bool Pump::togglePump(bool option) {
    // Don't do anything if the pump is in the requested state already
    if(option != pumpOn) {
        pumpOn = option;

        // Set the pin to the opposite state, since the pump is on the NC output from the relay
        digitalWrite(pumpPin, !pumpOn); // TODO: replace with RS485 command through Serial2
    }
    
    return pumpOn;
}

/*
 * Returns whether the pump is on.
 */
bool Pump::isPumpOn() {
    return pumpOn;
}