/****************************************************************************************************
 * pump.cpp
 * Carson Sloan
 * 
 * Defines the Pump class, which communicates with the pump using the ModbusMaster library.
/*****************************************************************************************************/

#include "pump.hpp"

/*
 * Creates a pump object with the provided controller saved.
 */
Pump::Pump(YAAJ_ModbusMaster _controller) {
    controller = _controller;

    // Check if the pump is already running, in case the ESP32 restarted during a routine or it was manually turned on
    checkStatus();
}

/*
 * Reads the pump status coil and returns whether the pump is currently running.
 */
bool Pump::checkStatus() {
    if (controller.F1_ReadCoils(0x1001, 1) == 0) {
        uint16_t state = controller.getRxBuf(0);
        Serial.printf("Pump status: %d\n", state);
        pumpOn = state != 0; // 0 = off, 1 = on
    }
    else {
        Serial.println("Error: Unable to read pump state!");
    }

    return pumpOn;
}

/*
 * Turns the pump on if true is provided, off if false. The state of the pump is returned afterward.
 */
bool Pump::setPump(bool option) {
    if (pumpOn != option) {
        pumpOn = option;

        uint16_t result = controller.F5_WriteSingleCoil(0x1001, pumpOn ? 0xFF : 0x00);
        if (result != 0) {
            Serial.printf("Unable to switch pump state! Error code: %d\n", result);
            pumpOn = !option;
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
 * Sets the speed of the pump to the requested value in ml/min. If a speed outside of 
 * the pump range (8-400) is requested, the pump will be set to the nearest speed in 
 * its range. The speed cannot be set when the pump is running, but the pump will be
 * turned off and back on if the force parameter is set. Whether the command was 
 * successful is returned.
 */
bool Pump::setSpeed(int flow, bool force/* = false*/) {
    // The pump will ignore speed commands when running
    if (pumpOn) {
        if (force) {
            setPump(false);
        }
        else {
            Serial.println("Error: Attempt to set pump speed while running.");
            return false;
        }
    }

    // Constrain values to within the pump range (although the pump controller does this as well for 0.1-420ish ml/min)
    if (flow < 8) {
        flow = 8;
    }
    else if (flow > 400) {
        flow = 400;
    }

    uint16_t low = 0;
    uint16_t high = 0;

    // TODO: currently only integer flow rates are possible, but the high bytes can be used to 
    // achieve decimal values, following the calculations done for flow rates over 256 ml/min

    // Start with the known register value at the edge of the precision level, then add the needed steps
    if (flow <= STEP_1) {
        low = STEP_0_CMD + ((flow - STEP_0) * RATE_0);
    }
    else if (flow > STEP_1 && flow <= STEP_2) {
        low = STEP_1_CMD + ((flow - STEP_1) * RATE_1);
    }
    else if (flow > STEP_2 && flow <= STEP_3) {
        low = STEP_2_CMD + ((flow - STEP_2) * RATE_2);
    }
    else if (flow > STEP_3 && flow <= STEP_4) {
        low = STEP_3_CMD + ((flow - STEP_3) * RATE_3);
    }
    else if (flow > STEP_4 && flow <= STEP_5) {
        low = STEP_4_CMD + ((flow - STEP_4) * RATE_4);
    }
    else if (flow > STEP_5) {
        low = STEP_5_CMD + (int) ((flow - STEP_5) * RATE_5);
        high = (flow % 2) * (0x8000); // add half of a step to achieve odd numbers
    }

    return setSpeed(high, low, force);
}

/*
 * Sends the command to write the provided 4 bytes to the speed holding registers in the pump 
 * controller. If the start parameter is set to true, starts the pump after speed is set. Returns
 * true if successful.
 */
bool Pump::setSpeed(uint16_t high, uint16_t low, bool start/* = false*/) {
    controller.setTxBuf(0, low);
    controller.setTxBuf(1, high);
    int result = controller.F16_WriteMultipleHoldingRegisters(0x3001, 0x02);

    if (start) {
        setPump(true);
    }

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