#include <Arduino.h>
#include "pump.hpp"

/*
 * Initialize the pump pin and turn the pump off.
 */
void Pump::init(ModbusMaster controller) {
    // Set the pump pin as output
    pinMode(PUMP_PIN, OUTPUT); // TODO: remove once Modbus control is implemented

    this->controller = controller;
    // TODO: consider scheduling this 0.5-1 sec later to make sure the pump is on
    //controller.writeSingleCoil(0x1004, true); // send the command to enable RS485 communication

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
        digitalWrite(PUMP_PIN, !pumpOn); // TODO: remove once Modbus control is implemented

        //uint_16t result = controller.writeSingleCoil(0x1001, !pumpOn); // setting this coil to a nonzero value turns the pump on
    }
    
    return pumpOn;
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