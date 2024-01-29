#include "routineManager.hpp"
#include "utils.hpp"
#include <Wire.h>

#define _TASK_TIMECRITICAL      // measure delay between scheduled time and actual start time
#define _TASK_STATUS_REQUEST    // use status requests to delay tasks until another has completed
#define _TASK_WDT_IDS           // for displaying control points and task IDs for debugging
#define _TASK_TIMEOUT           // a timeout can be set for when tasks should be deactivated
#include <TaskScheduler.h>

/*
 * Set the private pointers to the provided FlowManager and Pump objects, and run the test routine if requested.
 */
void RoutineManager::init(bool test) {
    Serial.begin(115200); // for USB debugging
    Serial2.begin(9600, SERIAL_8E1, MODBUS_RX, MODBUS_TX); // for pump control
    Wire.begin();
    while (!Serial || !Serial2) {} // wait until connections are ready

    controller.begin(0xC0, Serial2); // 0xC0 is the default pump address with all DIP switches off
    pinMode(MAX485_RE_NEG, OUTPUT);
    pinMode(MAX485_DE, OUTPUT);
    postTransmission(); // start in receive mode

    // These functions will be called automatically by the controller to make sure the MAX485 is configured correctly
    controller.preTransmission(RoutineManager::preTransmission);
    controller.postTransmission(RoutineManager::postTransmission);

    p = new Pump(controller);
    f = new FlowManager(p);

    controller.writeSingleCoil(0x1004, true); // send the command to enable RS485 communication

    if (test) {
        Event* head = buildTestRoutine();
        run(head);
        deleteRoutine(head);
    }
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
 * Runs the list of tasks linked to the provided head task pointer.
 */
void RoutineManager::run(Event* head) {
    // Loop through all tasks in the linked list
    while (head != NULL) {
        // Loop until the requested number of repetitions is reached
        for (int i = 0; i < head->getRepetitions(); i++) {
            // When a flow rate of 0 is requested, or this is an odd repetition in a cycle, turn off the pump
            if (head->getFlow() == 0 || i % 2 == 1) {
                p->setPump(false); // TODO: ensure the new pump can handle being toggled frequently
                delay(head->getOffDuration()); // TODO
            }
            else {
                p->setPump(true); // make sure the pump is on in all other cases
                // Try to achieve this flow rate with this flowManager
                f->setFlow(head->getFlow(), true);
                delay(head->getDuration()); // TODO: replace with a scheduled function call, and update the repetitions left
            }

            // TODO: check whether the target flow was reached in the duration requested and display a warning if not
            Serial.println("Task duration completed.");
        }

        // TODO: remove while loop, schedule a recursive call after duration for the next node instead
        head = head->getNext(); // do the next task, once it no longer exists the loop will exit
    }
}

/*
 * Recursively deletes all Tasks in the linked list starting at the provided Task*.
 */
void RoutineManager::deleteRoutine(Event* head) {
    deleteRoutine(head->getNext());

    if (head->getNext() == NULL) {
        delete head;
    }
}

/*
 * Set RE and DE pins to indicate a message will be sent.
 */
void RoutineManager::preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

/*
 * Reset RE and DE pins to indicate the device is ready to receive messages.
 */
void RoutineManager::postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}