#pragma once

#include "event.hpp"
#include "pump.hpp"
#include "flowManager.hpp"
#include <TaskSchedulerDeclarations.h> // this allows the reference to Scheduler while avoiding multiple declarations

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
};