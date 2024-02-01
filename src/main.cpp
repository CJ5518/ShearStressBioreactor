#include "routineManager.hpp"
#include "GUI.hpp"

RoutineManager rm;
Scheduler ts;
GUI gui;

void setup() {
    Serial.begin(115200); // for USB debugging
    //Serial.setDebugOutput(true);
    // Init RoutineManager
    rm.init(ts, false);
    // Init GUI, passing in RoutineManager instance pointer
    gui.init(nullptr);
    
}

void loop() {
    ts.execute();
}
