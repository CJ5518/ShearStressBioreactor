/****************************************************************************************************
 * main.cpp
 * 
 * Entry point to the program that initializes the RoutineManager, GUI and ThingSpeak connection
 * objects, as well as the Scheduler and ModbusMaster controller. The Arduino framework was used, 
 * with all function calls after setup() provided to the Scheduler as tasks to be run from loop().
/*****************************************************************************************************/

#include "routineManager.hpp"
#include "GUI.hpp"
#include "thingSpeak.hpp"

// Used for sending and receiving Modbus commands
HardwareSerial ModbusSerial(1);

// Receive and transmit pins for the MAX485
const int MODBUS_RX = 16;
const int MODBUS_TX = 17;
const int MODBUS_ENABLE = 18; // automatically set to high when writing, low otherwise to receive

const int PUMP_ADDRESS = 0xEF; // Modbus address of pump controller
const int MODBUS_TIMEOUT = 500; // timeout in ms for Modbus command responses

YAAJ_ModbusMaster controller;
RoutineManager rm;
Scheduler ts;
GUI gui;
ThingSpeak tsp;

/*
 * Initialize Serial connections and controller objects.
 */
void setup() {
    Serial.begin(115200); // for USB debugging
    // Serial connection for sending RS485 commands
    controller.begin(ModbusSerial, 9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX, PUMP_ADDRESS, MODBUS_ENABLE, MODBUS_TIMEOUT);
    while (!Serial || !ModbusSerial) {} // wait until connections are ready

    // ThingSpeak
    tsp.init();

    // RoutineManager initialization
    rm.init(&ts, true); // calls Scheduler::init()
    //rm.testControl(); // pump control testing
    //rm.collectFlowRates(); // testing for flow sensor data

    // Init GUI, passing in RoutineManager instance pointer
    gui.init(&ts, &rm);

    Serial.println("Setup complete.");
}

/*
 * Allow scheduler to execute the next task in the execution chain.
 */
void loop() {
    ts.execute();
}