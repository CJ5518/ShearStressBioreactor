/****************************************************************************************************
 * routineManager.hpp
 * Carson Sloan
 * 
 * Declares the RoutineManager class and member functions. In the future, most of the global
 * variables declared in routineManager.cpp should be moved to this class as private fields.
/****************************************************************************************************/

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
        RoutineManager(Scheduler* taskScheduler, bool test = false);
        ~RoutineManager();

        void init(Scheduler* ts, bool test);
        void run(Event* head);
        static void deleteRoutine(Event* head);

        // Test functions
        void collectFlowRates();
        void testControl();

        // GUI helper functions
        void setPump(bool on);
        bool isRunning();

        // Callback functions
        static void run();
        static void setFlow();
    private:
        Event* buildTestRoutine();
        static bool running;
        static void sendData(int speed);
};