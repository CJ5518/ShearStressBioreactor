#include "../include/routineManager.hpp"

RoutineManager rm;
Scheduler ts;

void setup() {
    // Init RoutineManager
    rm.init(ts, false);
    // Init GUI, passing in RoutineManager instance pointer?
}

void loop() {
    ts.execute();
}