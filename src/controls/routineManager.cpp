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
    Task lowFlow = Task(5.5f, 500); // spend 0.5 seconds at 5.5ml/min
    Task noFlow = Task(0.0f, 500); // spend 0.5 seconds with no flow
    noFlow.next = &lowFlow; // set the lowFlow task to come after noFlow is done

    Task first = Task(noFlow, 120); // repeat noFlow (and the attached lowFlow) 120 times
    first.next = &Task(200.0f, 12000); // after the first cycle is done, stay at 200ml/min for 2 minutes

    return &first;
}

/*
 * Runs the list of tasks linked to the provided head task pointer.
 */
void RoutineManager::run(Task* head) {
    while (head != NULL) {
        // Check if this is a cycle task
        if (head->getType() == TaskType::CYCLE) {
            for (int i = 0; i < head->getRepetitions(); i++) {
                run(head->getChild()); // recursively execute the child task
            }
        }
        else if (head->getType() == TaskType::FLOWRATE) {
            // When a flow rate of 0 is requested, turn off the pump
            if (head->getFlow() == 0) {
                p->togglePump(false);
            }
            else {
                p->togglePump(true); // make sure the pump is on in all other cases
            }

            // TODO still need to overhaul test_flow and implement duration checking!
            f->test_flow(head->getFlow(), true);
        }

        head = head->next; // do the next task, once it no longer exists the loop will exit
    }
}