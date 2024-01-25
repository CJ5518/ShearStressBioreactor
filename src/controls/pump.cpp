#include <Arduino.h>
#include "pump.hpp"

/*
 * Creates a pump object with the provided controller saved.
 */
Pump::Pump(ModbusMaster controller) {
    this->controller = controller;

    // The pump is off by default when power is provided
    pumpOn = false;
}

/*
 * Turns the pump on if true is provided, off if false. The state of the pump is returned afterward.
 */
bool Pump::setPump(bool option) {
    // Don't do anything if the pump is in the requested state already
    if(option != pumpOn) {
        pumpOn = option;

        uint16_t result = controller.writeSingleCoil(0x1001, pumpOn); // setting this coil to a nonzero value turns the pump on
        if (result != controller.ku8MBSuccess) {
            Serial.printf("Unable to switch pump state! Error code: %d\n", result);
        }
    }
    
    return pumpOn;
}

/*
 * Sets the pump to the opposite of its current state, and returns the new state.
 */
bool Pump::togglePump() {
    return setPump(!pumpOn);
}

/*
 * Sets the speed of the pump to the requested value. If a speed outside of the pump range is requested, the pump
 * will be set to the nearest speed in its range. The new speed is returned. 
 */
int Pump::setSpeed(int speed) {
    if (speed < 10) {
        speed = 10;
    }
    else if (speed > 400) {
        speed = 400;
    }

    // TODO: determine how speeds are encoded in the "Real-time speed" registers
    getSpeed();
    //controller.writeMultipleRegisters(0x3005, 400);

    return speed;
}

/*
 * Reads the speed from the pump controller's holding registers and prints them to the console. Returns whether the
 * request was successful.
 */
bool Pump::getSpeed() {
    uint16_t result = controller.readHoldingRegisters(0x3001, 0x6); // read all holding registers
    // Check if the error bit is 0
    if (result == controller.ku8MBSuccess) {
        // Print the stored speeds in rpm and resulting flow rate
        Serial.println("Current speed in rpm:");
        Serial.println(controller.getResponseBuffer(0x00)); // high 16 bits
        Serial.println(controller.getResponseBuffer(0x01)); // low 16 bits

        Serial.println("Current speed in ml/min:");
        Serial.println(controller.getResponseBuffer(0x04));
        Serial.println(controller.getResponseBuffer(0x05));

        return true;
    }
    
    return false;
}

/*
 * Returns whether the pump is on.
 */
bool Pump::isPumpOn() {
    return pumpOn;
}