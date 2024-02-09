#pragma once

#include "event.hpp"
#include "pump.hpp"
#include "flowManager.hpp"
#include <TaskSchedulerDeclarations.h> // this allows references to Scheduler while avoiding multiple declarations

class RoutineManager {
    public:
        RoutineManager() {}
        RoutineManager(Scheduler taskScheduler, bool test);

        void init(Scheduler ts, bool test);
        Event* buildTestRoutine();
        void run(Event* head);
        void deleteRoutine(Event* head);

        // Callback functions
        static void run();
        static void setFlow();

    private:
        // Receive and transmit pins for the MAX485
        const int MODBUS_RX = 16;
        const int MODBUS_TX = 17;
        const int MAX485_ENABLE = 18;

        // Receive and transmit pins for the second MAX485
        const int T_RX = 5;
        const int T_TX = 19;
        const int T_WRITE_ENABLE = 23;
};