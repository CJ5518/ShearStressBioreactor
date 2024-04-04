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

    //Variables used for error messages
    static void sendErrorMessage(AsyncWebServerRequest* request, int code, const String &message);

    //Initializes the server
    static void initServer(AsyncWebServer* server);

    //Tasks
    static Task* wifi_connect_task;
    static Task* wifi_watchdog_task;
    //Task callbacks
    static void wifi_connect_cb();
    static void wifi_watchdog_cb();

    static void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info);
};
