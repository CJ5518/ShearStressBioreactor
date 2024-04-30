/****************************************************************************************************
 * routineManager.cpp
 * Carson Sloan
 * 
 * Defines the RoutineManager class, which manages all physical bioreactor controls. The run(Event*)
 * function takes the head of a linked list of events defining the desired experiement to execute.
 * This is the only function that should be called in normal use cases, though several public test 
 * functions exist for validating flow rate reading and pump control. The init(Scheduler*) function
 * must be called before use.
/*****************************************************************************************************/

#define _TASK_TIMEOUT // allows a timeout to be set for when tasks should be stopped

#include "routineManager.hpp"
#include "utils.hpp"
#include "thingSpeak.hpp"
#include <TaskScheduler.h>
#include <Wire.h>

// TODO: move global static vars to private class members
static Scheduler *ts;
static bool offCycle = false;

bool RoutineManager::running;

static FlowManager* f;
static Pump* p;
static Event* head;

static Task* t1; // recurring call to run() that modifies t2
static Task* t2; // runs a callback to set the flow rate once

extern YAAJ_ModbusMaster controller;
extern ThingSpeak tsp;

/*
 * Default constructor is currently undefined. A Scheduler object should be provided via init().
 */
RoutineManager::RoutineManager() {
}

/* 
 * Calls init with the provided Scheduler object and test flag.
 */
RoutineManager::RoutineManager(Scheduler* taskScheduler, bool test/* = false*/) {
    init(taskScheduler, test);
}

/*
 * Destructor for RoutineManager object that deletes the saved routine.
 */
RoutineManager::~RoutineManager() {
    deleteRoutine(head);
}

/*
 * Set the private pointers to the provided FlowManager and Pump objects, and run the test routine if requested.
 */
void RoutineManager::init(Scheduler* taskScheduler, bool test) {
    Wire.begin(); // sda 21, scl 22 by default

    Serial.println("Completed setup, enabling RS485 communication.");
    
    // Wait until the command to enable RS485 communication is received
    while (controller.F5_WriteSingleCoil(0x1004, 0xFF) != 0) {
        Serial.println("Unable to establish RS485 communication!");
        delay(500);
    }

    Serial.println("Initializing pump and flow manager objects.");
    p = new Pump(controller);

    f = new FlowManager(p);
    ts = taskScheduler;
    ts->init();
    // Define the tasks to be reused for each routine step
    t1 = new Task(TASK_IMMEDIATE, TASK_ONCE, &run, ts, false); // call run()
    t2 = new Task(TASK_IMMEDIATE, TASK_ONCE, &setFlow, ts, false); // set specific flow rate
    running = false;

    // Run the test routine if requested
    if (test) {
        Event* head = buildTestRoutine();
        run(head);
    }
}

/*
 * Collects flow rate readings and prints them to the serial monitor, with an update on the average 
 * rate and sample period at the end of each loop.
 */
void RoutineManager::collectFlowRates() {
    unsigned long start;
    
    p->setSpeed(10);
    p->setPump(true);
    Serial.println("Priming system...");
    delay(1000);

    //int speeds[10] = {10, 30, 15, 25, 20, 10, 30, 15, 25, 20}; //{40, 80, 120, 160, 80, 40, 80, 120, 160, 80};
    //int speeds[5] = {100, 150, 200, 150, 100};

    for (int i = 0; i < 20; i++) {
        start = millis();
        int speed = (10 * i) + 10;
        p->setSpeed(speed);
        Serial.printf("Pump set to %d ml/min\n", speed);
        //delay(100);
        Serial.printf("Average flow rate: %.5f ml/min, measured in %d ms\n", f->takeAvgNumReadings(speed <= 40, 500, true), millis() - start);
    }

    p->setPump(false);
}

/*
 * Tests a range of pump speed settings to help determine the conversion to ml/min.
 */
void RoutineManager::testControl() {
    p->getSpeed(true);
    delay(500);

    for (int i = 0; i < 400; i++) {
        p->setSpeed(i);
        int32_t speed = p->getSpeed(true);
        if (speed < 0) {
            Serial.println("Unable to read flow rate setting.");
        }
        delay(100);
    }
}

/*
 * Manually sets the pump state to the provided value (true = on), to allow manual control from the GUI (like for cleaning).
 */
void RoutineManager::setPump(bool on) {
    p->setPump(on);
}

/*
 * Builds an example routine and returns the head task pointer.
 */
Event* RoutineManager::buildTestRoutine() {
    // Spend 0.5 seconds at 5.5 ml/min, then 0.5 seconds off
    Event* lowFlow1 = new Event(30.0f, 20000, 15, 20000); // the complete constructor could be used also: Task(5.5f, 500, 1, 500)
    Event* lowFlow2 = new Event(40.0f, 20000, 15, 20000);
    lowFlow1->setNext(lowFlow2);
    /*Event* lowFlow3 = new Event(15.5f, 15000); // stay at 15.5 ml/min for 1 sec (no off cycle)
    lowFlow2->setNext(lowFlow3);
    Event* lowFlow4 = new Event(17.5f, 15000, 3); // move to 17.5 ml/min for 1 sec
    lowFlow3->setNext(lowFlow4);

    // After the low flow rates are done, stay at 200ml/min for 10 sec
    Event* highFlow1 = new Event(20.0f, 30000); // could be written as Task(200.0f, 12000, 1, 0)
    lowFlow1->append(highFlow1); // append to the end of the list instead of setting next for lowFlow4
    Event* highFlow2 = new Event(30.0f, 15000, 5, 15000); // move to 300 ml/min for 1 sec, then off for 2 sec, 5 times
    lowFlow3->append(highFlow2); // append can be called for any task in the list, not just the first or last, but the length will appear shorter
    Event* highFlow3 = new Event(25.0f, 15000, 1);
    int count = lowFlow1->append(highFlow3); // append returns the new length of the list starting from lowFlow1
    Serial.printf("Number of tasks in the list: %d\n", count);*/

    return lowFlow1;
}

/*
 * Returns true if a routine is currently being run, to allow the GUI to display the current status.
 */
bool RoutineManager::isRunning() {
    return running;
}

/*
 * Saves the provided linked list of events, and calls the run() function to begin scheduing of events.
 */
void RoutineManager::run(Event* newHead) {
    tsp.sendToThingSpeak_field7(0);
    tsp.sendToThingSpeak_field7(1);
    running = true;

    head = newHead;
    t1->setIterations(1);

    // Make sure the pump starts off
    if (p->checkStatus()) {
        p->setPump(false);
        t1->enableDelayed(2000); // allow the flow to stop
    }
    else {
        t1->enable();
    }
}

/*
 * Recursively schedules the events contained in the linked list starting with static member head.
 * This function is for use only as a task callback; routines must be started with a call to run(Event*).
 */
void RoutineManager::run() {
    //sendData(head->getFlow());

    // Decrement the number of repetitions left, and check if it was 0
    if (head->decRepetitions() == 0) {
        head = head->getNext(); // move to the next event
        offCycle = false; // reset in case next event is a cycle

        if (head == NULL) {
            Serial.println("Routine execution has finished.");
            running = false;
            tsp.sendToThingSpeak_field7(1);
            tsp.sendToThingSpeak_field7(0);
            p->setPump(false);
            t1->disable();
            deleteRoutine(head);
            return;
        }
    }

    Serial.printf("Running event: flow rate: %.2f, duration: %d, \
        off duration: %d, cycles left: %d\n", head->getFlow(), 
        head->getDuration(), head->getOffDuration(), head->getRepetitions());

    // When a flow rate of 0 is requested, or this is an odd repetition in a cycle with specified offDuration, turn off the pump
    if (head->getFlow() == 0 || offCycle) {
        p->setPump(false);
        tsp.sendToThingSpeak_field3(0); // 5 for pump speed
    }
    else {
        // Try to achieve this flow rate with the flowManager, with a timeout set
        t2->setTimeout(head->getDuration() - 1);
        t2->setIterations(1);
        t2->enable(); // will execute setFlow as soon as run() returns
    }

    // If an offDuration has been set for this Event, and the last call did not turn off the pump, set the static flag for the next call to run()
    if (head->getOffDuration() > 0 && !offCycle) {
        offCycle = true;
    }
    else {
        offCycle = false; // if this event has no offDuration, or the last call turned off the pump, turn it on this time
    }

    t1->setIterations(1);
    // Schedule the next task for after the requested duration for this flow rate
    t1->enableDelayed(head->getDuration());
}

/*
 * Callback-friendly function that calls FlowManager::setFlow with the head event's flow rate.
 */
void RoutineManager::setFlow() {
    if (head != NULL) {
        //f->setFlow(head->getFlow());
        int rate = (int) head->getFlow();
        p->setSpeed(rate);
        p->setPump(true);
        
        sendData(rate);
    }
    else {
        Serial.println("Error: setFlow() called with no Event object in routine list.");
    }
}

/*
 * Calls the relevant functions to send data to thingspeak. Unfortunately, with the free plan only 1 value 
 * can be sent every 15 seconds.
 */
void RoutineManager::sendData(int speed) {
    //tsp.getWifiAndLed();
    //tsp.sendToThingSpeak_field7(1);
    //tsp.sendToThingSpeak_field4(speed);
    //tsp.sendToThingSpeak_field5(speed);
    double actual = f->takeAvgNumReadings(speed <= 40, 50, true);
    tsp.sendToThingSpeak_field3(actual);
    //Utils::reynolds(actual);
    //Utils::shearStress(speed);
    //Serial.printf("Shear stress: %.2f\n", Utils::shearStress(actual));
}

/*
 * Recursively deletes all Tasks in the linked list starting at the provided Task*.
 */
void RoutineManager::deleteRoutine(Event* node) {
    if (node == NULL) {
        return;
    }

    deleteRoutine(node->getNext());

    if (node->getNext() == NULL) {
        delete node;
    }
}