#pragma once

#include "event.hpp"
#include "pump.hpp"
#include "flowManager.hpp"
#include <HardwareSerial.h>
#include <YAAJ_ModbusMaster.h>
#include <TaskSchedulerDeclarations.h> // this allows references to Scheduler while avoiding multiple declarations

class RoutineManager {
    public:
        RoutineManager();
        RoutineManager(Scheduler* taskScheduler, bool test);

        void init(Scheduler* ts, bool test);
        Event* buildTestRoutine();
        void run(Event* head);
        void deleteRoutine(Event* head);

        void collectFlowRates();
        void testControl();

        // Callback functions
        static void run();
        static void setFlow();
};