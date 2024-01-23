#include <Arduino.h>
#include "../include/projectHeaders.hpp"
#include <Wire.h>
#include <ModbusMaster.h>

const bool IS_WATER = true; // whether the flow sensors should measure with water calibration or Isopropyl
// Receive and transmit pins for the MAX485
const int MODBUS_RX = 16;
const int MODBUS_TX = 17;
const int MAX485_DE = 18;
const int MAX485_RE_NEG = 19;

bool debug = false; // debug mode

Pump p;
FlowManager fm;
RoutineManager rm;
ModbusMaster controller;

/*
 * Set RE and DE pins to indicate a message will be sent.
 */
void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

/*
 * Reset RE and DE pins to indicate the device is ready to receive messages.
 */
void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

/*
 * Start serial connection and initialize the pump and flow manager.
 */
void setup() {
    Serial.begin(9600); // for USB debugging
    Serial2.begin(9600, SERIAL_8E1, MODBUS_RX, MODBUS_TX); // for pump control
    Wire.begin();
    while (!Serial || !Serial2) {} // wait until connections are ready

    controller.begin(0xC0, Serial2); // 0xC0 is the default pump address with all DIP switches off
    //pinMode(MAX485_RE_NEG, OUTPUT);
    //pinMode(MAX485_DE, OUTPUT);
    //postTransmission(); // start in receive mode

    // These functions will be called automatically by the controller to make sure the MAX485 is configured correctly
    controller.preTransmission(preTransmission);
    controller.postTransmission(postTransmission);

    p.init(controller); // initialize the pump and pass the controller object
    fm.init(&p); // initialize flow manager with pump pointer so flow rates can be adjusted by the pump as well
    rm.init(&fm, &p, false); // TODO: eventually only the routine manager needs to control the flow manager and maybe pump too

    delay(50);
    if (debug) {
        rm.run(rm.buildTestRoutine());
    }
}

/*
 * TODO: deal with scheduling
 */
void loop() {
    delay(1000);
    p.togglePump(false);
    delay(1000);
    p.togglePump(true);
}