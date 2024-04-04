#include "routineManager.hpp"
#include "GUI.hpp"

// Used for sending and receiving Modbus commands
HardwareSerial ModbusSerial(1);

// Receive and transmit pins for the MAX485
const int MODBUS_RX = 16;
const int MODBUS_TX = 17;
const int MODBUS_ENABLE = 18; // automatically set to high when writing, low otherwise to receive

YAAJ_ModbusMaster controller;

RoutineManager rm;
Scheduler ts;
GUI gui;

void setup() {
    Serial.begin(115200); // for USB debugging
    // Serial connection for sending RS485 commands
    controller.begin(ModbusSerial, 9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX, 0xEF, MODBUS_ENABLE, 500);
    while (!Serial || !ModbusSerial) {} // wait until connections are ready

    // RoutineManager initialization
    rm.init(&ts, false);
    //rm.testControl(); // pump control testing
    rm.collectFlowRates(); // testing for flow sensor data

    // Init GUI, passing in RoutineManager instance pointer
    gui.init(&ts, &rm);
    Serial.println("Setup complete.");
}

void loop() {
    ts.execute();
}