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
        void run(Event* head);
        static void deleteRoutine(Event* head);
        static void sendData(int speed);

        void collectFlowRates();
        void testControl();
        void setPump(bool on);

        bool isRunning();

        // Callback functions
        static void run();
        static void setFlow();
    private:
        Event* buildTestRoutine();
        static bool running;
};