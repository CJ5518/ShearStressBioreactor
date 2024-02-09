#include <Arduino.h>
#include "pump.hpp"

/*
 * Creates a pump object with the provided controller saved.
 */
Pump::Pump(YAAJ_ModbusMaster _controller) {
    controller = _controller;

    // The pump is off by default when power is provided
    pumpOn = false;
}

/*
 * Turns the pump on if true is provided, off if false. The state of the pump is returned afterward.
 */
bool Pump::setPump(bool option) {
    pumpOn = option;

    uint16_t result = controller.F5_WriteSingleCoil(0x1001, pumpOn ? 0xFF : 0x00);
    if (result != 0) {
        Serial.printf("Unable to switch pump state! Error code: %d\n", result);
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
 * Sets the speed of the pump to the requested value in ml/min. If a speed outside of the pump range (10-400) is requested,
 * the pump will be set to the nearest speed in its range. Whether the command was successful is returned.
 */
bool Pump::setSpeed(int flow) {
    // Contstrain values to within the pump range (although the pump controller does this as well for 0.1-420ish ml/min)
    if (flow < 10) {
        flow = 10;
    }
    else if (flow > 400) {
        flow = 400;
    }

    uint16_t low = 0;
    int min = 0;
    int rate = 0;

    // TODO: verify accuracy for requested rates close to a step
    // Start with the known register value at the edge of the precision level, then add the needed steps
    if (flow < STEP_1) {
        low = STEP_0_CMD + flow * RATE_0;
    }
    else if (flow >= STEP_1 && flow < STEP_2) {
        low = STEP_1_CMD + ((flow - STEP_1) * RATE_1);
    }
    else if (flow >= STEP_2 && flow < STEP_3) {
        low = STEP_2_CMD + ((flow - STEP_2) * RATE_2);
    }
    else if (flow >= STEP_3) {
        low = STEP_3_CMD + ((flow - STEP_3) * RATE_3);
    }

    return setSpeed(0, low);
}

/*
 * Sends the command to write the provided 4 bytes to the speed holding registers in the pump controller. Returns true if successful.
 */
bool Pump::setSpeed(uint16_t high, uint16_t low) {
    // TODO: determine whether this is switched
    controller.setTxBuf(0, low);
    controller.setTxBuf(1, high);
    int result = controller.F16_WriteMultipleHoldingRegisters(0x3001, 0x02);

    if (result != 0) {
        Serial.printf("Error (%d) setting flow rate!\n", result);
        return false;
    }
    else {
        return true;
    }
}

/*
 * Reads the speed from the pump controller's holding registers and prints them to the console if requested. Returns the
 * low 2 bytes of the speed holding registers, or -1 if there was no response.
 */
int32_t Pump::getSpeed(bool print) {
    uint16_t result = controller.F3_ReadMultipleHoldingRegisters(0x3001, 6); // read all holding registers
    int32_t lowBytes = -1;
    // Check if the command returned no error
    if (print && result == 0) {
        // Print the speeds stored in the holding registers
        lowBytes = controller.getRxBuf(0);
        Serial.printf("Set speed: %X %X\n", controller.getRxBuf(1), lowBytes);
        Serial.printf("Real-time speed: %X %X\n", controller.getRxBuf(5), controller.getRxBuf(4));
    }
    
    return lowBytes;
}

/*
 * Returns whether the pump is on.
 */
bool Pump::isPumpOn() {
    return pumpOn;
}