#include "routineManager.hpp"
#include "GUI.hpp"

// Used for sending and receiving Modbus commands
HardwareSerial tSerial1(1);
HardwareSerial tSerial2(2);

YAAJ_ModbusMaster controller;

RoutineManager rm;
Scheduler ts;
GUI gui;

// Receive and transmit pins for the MAX485
const int MODBUS_RX = 16;
const int MODBUS_TX = 32; //17;
const int MODBUS_ENABLE = 18;

// Receive and transmit pins for the second MAX485
const int T_RX = 5;
const int T_TX = 19;
const int T_WRITE_ENABLE = 23;

void setup() {
    Serial.begin(115200); // for USB debugging

    //Serial.begin(9600); // USB debugging
    //tSerial1.begin(9600, SERIAL_8N1, T_RX_1, T_TX_1); // default sender, begun by controller.begin
    //Wire.begin();


    //Serial.printf("Address: %X Function: %X Coil: %X %X Value: %X %X CRC: %X %X\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

        //Serial.setDebugOutput(true);
        // Init RoutineManager
        //rm.init(ts, false);
        // Init GUI, passing in RoutineManager instance pointer
        //gui.init(&ts, &rm);
        rm.hardware(tSerial1);
        rm.testControl(tSerial1, tSerial2);
        rm.init(ts, false);
}
