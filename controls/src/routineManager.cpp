#include "routineManager.hpp"

/*
 * Set the private pointers to the provided FlowManager and Pump objects, and run the test routine if requested.
 */
void RoutineManager::init(FlowManager* fm, Pump* pump, bool test) {
    f = fm;
    p = pump;

    if (test) {
        Task* head = buildTestRoutine();
        run(head);
    }
}

/*
 * Builds an example routine and returns the head task pointer. The routine alternates between 5.5 and 0ml/min at 2Hz
 * for 2 minutes, then maintains 200ml/min for 2 minutes.
 */
Task* RoutineManager::buildTestRoutine() {
    // Spend 0.5 seconds at 5.5 ml/min, then 0.5 seconds off
    Task lowFlow1(5.5f, 500, 1); // the complete constructor could be used like Task(5.5f, 500, 120, 500)
    Task lowFlow2(7.5f, 500, 1);
    lowFlow1.next = &lowFlow2;
    Task lowFlow3(15.5f, 1000); // stay at 15.5 ml/min for 1 sec (no off cycle)
    lowFlow2.next = &lowFlow3;
    Task lowFlow4(17.5f, 1000); // move to 17.5 ml/min for 1 sec
    lowFlow3.next = &lowFlow4;

    // After the low flow rates are done, stay at 200ml/min for 10 sec
    Task highFlow1(200.0f, 10000); // could be written as Task(200.0f, 12000, 1, 0)
    lowFlow4.next = &highFlow1;
    Task highFlow2(300.0f, 1000, 5, 2000); // move to 300 ml/min for 1 sec, then off for 2 sec, 5 times
    highFlow1.next = &highFlow2;

    return &lowFlow1;
}

/*
 * Runs the list of tasks linked to the provided head task pointer.
 */
void RoutineManager::run(Task* head) {
    // Loop through all tasks in the linked list
    while (head != NULL) {
        // Loop until the requested number of repetitions is reached
        for (int i = 0; i < head->getRepetitions(); i++) {
            // When a flow rate of 0 is requested, or this is an odd repetition in a cycle, turn off the pump
            if (head->getFlow() == 0 || i % 2 == 1) {
                p->togglePump(false); // TODO: ensure the new pump can handle being toggled frequently
                delay(head->getOffDuration()); // TODO
            }
            else {
                p->togglePump(true); // make sure the pump is on in all other cases
                // Try to achieve this flow rate with this flowManager
                f->setFlow(head->getFlow(), true);
                delay(head->getDuration()); // TODO: replace with a scheduled function call, and update the repetitions left
            }

            // TODO: check whether the target flow was reached in the duration requested and display a warning if not
            Serial.println("Task duration completed.");
        }

        // TODO: remove while loop, schedule a recursive call after duration for the next node instead
        head = head->next; // do the next task, once it no longer exists the loop will exit
    }
}