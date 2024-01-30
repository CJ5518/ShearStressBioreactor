#pragma once

#include "event.hpp"
#include "pump.hpp"
#include "flowManager.hpp"

#define _TASK_TIMECRITICAL      // measure delay between scheduled time and actual start time
#define _TASK_STATUS_REQUEST    // use status requests to delay tasks until another has completed
#define _TASK_WDT_IDS           // for displaying control points and task IDs for debugging
#define _TASK_TIMEOUT           // a timeout can be set for when tasks should be deactivated
#include <TaskScheduler.h>

class RoutineManager {
    public:
        void init(Scheduler ts, bool test);
        Event* buildTestRoutine();
        void run(Event* head);
        void deleteRoutine(Event* head);

        // Callback functions
        static void run();
        static void setFlow();
        static void postTransmission();
        static void preTransmission();

    private:
        // Receive and transmit pins for the MAX485
        const int MODBUS_RX = 16;
        const int MODBUS_TX = 17;
        static const int MAX485_DE = 18;
        static const int MAX485_RE_NEG = 19;

        static bool offCycle;

        static FlowManager* f;
        static Pump* p;
        static ModbusMaster controller;
        static Scheduler ts;
        static Event* head;
};