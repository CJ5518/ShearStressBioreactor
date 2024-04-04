#define _TASK_TIMECRITICAL      // measure delay between scheduled time and actual start time
//#define _TASK_STATUS_REQUEST    // use status requests to delay tasks until another has completed
//#define _TASK_WDT_IDS           // for displaying control points and task IDs for debugging
#define _TASK_TIMEOUT           // a timeout can be set for when tasks should be deactivated

#include "routineManager.hpp"
#include "utils.hpp"
#include <TaskScheduler.h>
#include <Wire.h>

// TODO: move global static vars to private class members
static Scheduler *ts;
static bool offCycle;

static FlowManager* f;
static Pump* p;
static Event* head;

static Task t1; // recurring call to run() that modifies t2
static Task* t2; // runs a callback to set the flow rate once

extern YAAJ_ModbusMaster controller;

/*
 * Default constructor is currently undefined. A Scheduler object should be provided via init().
 */
RoutineManager::RoutineManager() {
}

/* 
 * Calls init with the provided Scheduler object and test flag.
 */
RoutineManager::RoutineManager(Scheduler* taskScheduler, bool test) {
    init(taskScheduler, test);
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
        delay(300);
    }

    Serial.println("Initializing pump and flow manager objects.");
    p = new Pump(controller);
    f = new FlowManager(p);
    ts = taskScheduler;
    t2 = new Task(TASK_IMMEDIATE, TASK_ONCE, &setFlow, ts, true);

    // Run the test routine if requested
    if (test) {
        Event* head = buildTestRoutine();
        run(head);
        deleteRoutine(head);
    }
}

/*
 * Collects 2000 flow rate readings and prints them to the serial monitor, with an update on the average rate and sample period every 1000.
 */
void RoutineManager::collectFlowRates() {
    unsigned long start;

    for (int i = 0; i < 2; i++) {
        start = millis();
        Serial.printf("Average flow rate: %.5fml/min, measured in %dms\n", f->takeAvgNumReadings(true, 1000), millis() - start);
    }
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
 * Manually sets the pump state to the provided value (true = on).
 */
void RoutineManager::setPump(bool on) {
    p->setPump(on);
}

/*
 * Builds an example routine and returns the head task pointer.
 */
Event* RoutineManager::buildTestRoutine() {
    // Spend 0.5 seconds at 5.5 ml/min, then 0.5 seconds off
    Event* lowFlow1 = new Event(5.5f, 500, 1); // the complete constructor could be used also: Task(5.5f, 500, 1, 500)
    Event* lowFlow2 = new Event(7.5f, 500, 1);
    lowFlow1->setNext(lowFlow2);
    Event* lowFlow3 = new Event(15.5f, 1000); // stay at 15.5 ml/min for 1 sec (no off cycle)
    lowFlow2->setNext(lowFlow3);
    Event* lowFlow4 = new Event(17.5f, 1000); // move to 17.5 ml/min for 1 sec
    lowFlow3->setNext(lowFlow4);

    // After the low flow rates are done, stay at 200ml/min for 10 sec
    Event* highFlow1 = new Event(200.0f, 10000); // could be written as Task(200.0f, 12000, 1, 0)
    lowFlow1->append(highFlow1); // append to the end of the list instead of setting next for lowFlow4
    Event* highFlow2 = new Event(300.0f, 1000, 5, 2000); // move to 300 ml/min for 1 sec, then off for 2 sec, 5 times
    lowFlow3->append(highFlow2); // append can be called for any task in the list, not just the first or last, but the length will appear shorter
    Event* highFlow3 = new Event(400.0f, 1000, 1);
    int count = lowFlow1->append(highFlow3); // append returns the new length of the list starting from lowFlow1
    Serial.printf("Number of tasks in the list: %d\n", count);

    return lowFlow1;
}

/*
 * Saves the provided linked list of events, and calls the run() function to begin scheduing of events.
 */
void RoutineManager::run(Event* newHead) {
    head = newHead;
    run();
}

/*
 * Recursively schedules the events contained in the linked list starting with static member head.
 */
void RoutineManager::run() {
    // Decrement the number of repetitions left, and check if it was 0
    if (head->decRepetitions() == 0) {
        head = head->getNext(); // move to the next event

        if (head == NULL) {
            Serial.println("Routine execution has finished.");
            ts->disableAll();
            return;
        }
    }

    t1.setCallback(&run); // prepare a future recursive call to run

    // If an offDuration has been set for this Event, and the last call did not turn off the pump, set the static flag for the next call to run()
    if (head->getOffDuration() > 0 && !offCycle) {
        offCycle = true;
    }
    else {
        offCycle = false; // if this event has no offDuration, or the last call turned off the pump, turn it on this time
    }

    // When a flow rate of 0 is requested, or this is an odd repetition in a cycle with specified offDuration, turn off the pump
    if (head->getFlow() == 0 || offCycle) {
        p->setPump(false);
        t1.delay(head->getOffDuration());
    }
    else {
        p->setPump(true); // make sure the pump is on in all other cases
        // Try to achieve this flow rate with the flowManager, with a timeout set
        t2->setTimeout(head->getDuration() - 10);
        // Schedule the next task for after the requested duration for this flow rate
        t1.delay(head->getDuration());
    }

    ts->addTask(t1);
    t1.enableDelayed();
}

/*
 * Callback-friendly function that calls FlowManager::setFlow with the head event's flow rate.
 */
void RoutineManager::setFlow() {
    f->setFlow(head->getFlow());
}

/*
 * Recursively deletes all Tasks in the linked list starting at the provided Task*.
 */
void RoutineManager::deleteRoutine(Event* node) {
    deleteRoutine(node->getNext());

    if (node->getNext() == NULL) {
        delete node;
    }
}