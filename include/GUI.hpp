#pragma once

#include "routineManager.hpp"
#include "ESPAsyncWebServer.h"
#include <TaskSchedulerDeclarations.h> // this allows the reference to Scheduler while avoiding multiple declarations


//Class encapsulating the GUI of the program
//Contains code and data for setting up the web server and interacting with the routine manager
//from the html interface
class GUI {
public:
    GUI();
    //Inits the server in the constructor
    GUI(RoutineManager* rm);
    //Inits the server
    void init(RoutineManager* rm);
    //Closes all server connections and deletes the server,
    //recommend to call this to make sure the connections close cleanly
    void end();

private:
    //Pointer to external routine manager
    RoutineManager* routineManager;
    //Pointer to internal web server
    AsyncWebServer* server;
};
