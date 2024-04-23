#pragma once

#include "routineManager.hpp"
#include "ESPAsyncWebServer.h"
#include <TaskSchedulerDeclarations.h>


//Class encapsulating the GUI of the program
//Contains code and data for setting up the web server and interacting with the routine manager
//from the html interface
class GUI {
public:
    //Inits the server
    static void init(Scheduler* ts, RoutineManager* rm);
    //Closes all server connections and deletes the server,
    //recommend to call this to make sure the connections close cleanly
    static void end();

    //Only called by GUI.cpp
    static void initWebServer();


//private:
    //Pointer to external routine manager
    static RoutineManager* routineManager;
    //Pointer to internal web server
    static AsyncWebServer* server;
    static Scheduler* taskScheduler;

    static void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info);
    static void initServer(AsyncWebServer* server);
};
