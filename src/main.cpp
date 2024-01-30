// Future home of the program's entry point

#include "../include/routineManager.hpp"

RoutineManager rm;
Scheduler ts;

void setup() {
    // Init RoutineManager
    rm.init(ts, false);
    // Init GUI, passing in RoutineManager instance pointer?

}

// Hopefully not needed once freeRTOS is implemented
void loop() {
    ts.execute();
}