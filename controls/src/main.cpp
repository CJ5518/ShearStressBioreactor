#include <Arduino.h>
#include "../include/projectHeaders.hpp"
#include <Wire.h>

const bool isWater = true;
bool debug = false; // debug mode

Pump p;
FlowManager fm;
RoutineManager rm;

/*
 * Start serial connection and initialize the pump and flow manager.
 */
void setup() {
    Serial.begin(9600);
    Wire.begin();
    while (!Serial) {} // wait until connection is ready

    p.init(); // initialize the pump
    fm.init(); // initialize flow manager
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
    
    delay(100);
}
