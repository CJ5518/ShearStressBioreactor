#define _TASK_TIMECRITICAL      // measure delay between scheduled time and actual start time
//#define _TASK_STATUS_REQUEST    // use status requests to delay tasks until another has completed
//#define _TASK_WDT_IDS           // for displaying control points and task IDs for debugging
#define _TASK_TIMEOUT           // a timeout can be set for when tasks should be deactivated

#include <TaskScheduler.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "routineManager.hpp"
#include "utils.hpp"

static Scheduler ts;
static bool offCycle;

static FlowManager* f;
static Pump* p;
static YAAJ_ModbusMaster controller;
static Event* head;

static HardwareSerial modbusSerial(1);
static HardwareSerial tSerial(2);

/* 
 * Calls init with the provided Scheduler object and test flag.
 */
RoutineManager::RoutineManager(Scheduler taskScheduler, bool test) {
    init(taskScheduler, test);
}

/*
 * Set the private pointers to the provided FlowManager and Pump objects, and run the test routine if requested.
 */
void RoutineManager::init(Scheduler taskScheduler, bool test) {
    tSerial.begin(9600, SERIAL_8N1, T_RX, T_TX); // sniff all messages sent on RS485 bus
    controller.begin(modbusSerial, 9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX, 0xEF, MAX485_ENABLE, 200);
    Wire.begin();
    while (!Serial || !tSerial || !modbusSerial) {} // wait until connections are ready

    pinMode(MAX485_ENABLE, OUTPUT);
    pinMode(T_WRITE_ENABLE, OUTPUT);

    // Start in receive mode
    digitalWrite(T_WRITE_ENABLE, LOW);
    digitalWrite(MAX485_ENABLE, LOW);
    
    p = new Pump(controller);
    f = new FlowManager(p);
    ts = taskScheduler;

    Serial.println("Completed setup, enabling RS485 communication.");
    bool connected = false;
    char* data = (char*) malloc(8 * sizeof(char));

    while (!connected) {
        // Send the command to enable RS485 communication
        uint16_t result = controller.F5_WriteSingleCoil(0x1004, 0xFF);
        unsigned long start = millis();

        while (millis() - start < 1000) {
            if (result != 0) {
                Serial.printf("Unable to establish RS485 communication! Error code: %d\n", result);

                // Read all data on the RS485 bus
                if (tSerial.available()) {
                    tSerial.readBytes(data, 8);
                    Serial.printf("Address: %X Function: %X Coil: %X %X Value: %X %X CRC: %X %X\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                }
            }
            else {
                connected = true;
            }
        }
    }

    /*//p->setPump(true);
    // 41F0 = 30ml/min
    // 43C7 = 398ml/min
    // 258 = 438B = transition 2->1
    // 127.5 = 4309 = transition from 1->.5
    // 63.2 = 4287 = transition from .5->.25
    p->setSpeed(0, 0x43C7);
    delay(3000);
    while (true) {
        int32_t speed = p->getSpeed(true);
        if (speed < 0) {
            Serial.println("Unable to read flow rate setting.");
        }
        //p->togglePump();
        p->setSpeed(0, speed -= 10);
        delay(2000);
    }*/

    if (test) {
        Event* head = buildTestRoutine();
        run(head);
        deleteRoutine(head);
    }

    /*// Testing for flow sensor data
    unsigned long start;

    for (int i = 0; i < 1000; i++) {
        start = millis();
        Serial.printf("Average flow rate: %.5fml/min, measured in %dms\n", f->takeAvgNumReadings(true, 1000), millis() - start);
    }*/
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
            return;
        }
    }

    // TODO: ensure task initialization happens in a global scope
    Task t1; // initialize the next task
    t1.setCallback(&run);

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
        Task t2(TASK_IMMEDIATE, TASK_ONCE, &setFlow, &ts, true);
        // Schedule the next task for after the requested duration for this flow rate
        t1.delay(head->getDuration());
    }

    ts.addTask(t1);
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