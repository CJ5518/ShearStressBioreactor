#include "task.hpp"

/*
 * Constructor for a constant flow rate specifier task that sets the target flow rate and duration to the provided values.
 * The number of repetitions is set to 1 by default, and the offDuration is set to 0.
 */
Task::Task(float newFlow, int newDuration) {
    flow = newFlow;
    duration = newDuration;
    repetitions = 1;
    offDuration = 0;
    next = NULL;
}

/*
 * Constructor for a cycle task that sets the flow rate, duration, and number of repetitions to the provided values. The off
 * duration is set to the provided on duration as well.
 */
Task::Task(float newFlow, int newDuration, int newRepetitions) {
    flow = newFlow;
    duration = newDuration;
    offDuration = duration; // stay off and on for the same amount of time
    repetitions = newRepetitions;
    next = NULL;
}

/*
 * Constructor for a cycle task that sets the flow rate, duration, number of repetitions and duration to stay off
 * to the provided values.
 */
Task::Task(float newFlow, int newDuration, int newRepetitions, int newOffDuration) {
    flow = newFlow;
    duration = newDuration;
    offDuration = newOffDuration;
    repetitions = newRepetitions;
    next = NULL;
}

/*
 * Sets the flow rate member to the provided value and returns the old value.
 */
float Task::setFlow(float newFlow) {
    int old = flow;
    flow = newFlow;
    return old;
}

/*
 * Sets the repetitions member to the provided value and returns the old value.
 */
int Task::setRepetitions(int newRepetitions) {
    int old = repetitions;
    repetitions = newRepetitions;
    return old;
}

/*
 * Decrements the repetitions member by 1 and returns the old value.
 */
int Task::decRepetitions() {
    return repetitions--;
}

/*
 * Sets the duration of this task to the provided value and returns the old value.
 */
int Task::setDuration(int newDuration) {
    int old = duration;
    duration = newDuration;
    return old;
}

/*
 * Sets the off duration of this task to the provided value and returns the old value.
 */
int Task::setOffDuration(int newDuration) {
    int old = offDuration;
    offDuration = newDuration;
    return old;
}

/*
 * Returns the target flow rate of this task.
 */
float Task::getFlow() {
    return flow;
}

/*
 * Returns the repetitions of this task if it is a cycle.
 */
int Task::getRepetitions() {
    return repetitions;
}

/*
 * Returns the value of the duration member of this task.
 */
int Task::getDuration() {
    return duration;
}

/*
 * Returns the duration this task will stay off.
 */
int Task::getOffDuration() {
    return offDuration;
}

/*
 * Returns a pointer to the next task in the linked list.
 */
Task* Task::getNext() {
    return next;
}