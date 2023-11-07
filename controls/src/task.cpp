#include "task.hpp"

/*
 * Constructor for a flow rate specifier task that sets the target flow rate and duration to the provided values.
 */
Task::Task(float newFlow, int newDuration) {
    type = TaskType::FLOWRATE;
    attr.flow = newFlow;
    duration = newDuration;
    child = NULL;
    next = NULL;
}

/*
 * Constructor for a cycle task that sets the child task and number of repetitions to the provided values.
 */
Task::Task(Task newChild, int newRepetitions) {
    type = TaskType::CYCLE;
    attr.repetitions = newRepetitions;
    child = &newChild;
    next = NULL;
}

/*
 * Sets the repetitions member to the provided value and returns the old value. If this task is a cycle,
 * no flow rate should be provided so -1 is returned.
 */
float Task::setFlow(float newFlow) {
    if (type == TaskType::CYCLE) {
        return -1;
    }

    int old = attr.flow;
    attr.flow = newFlow;
    return old;
}

/*
 * Sets the repetitions member to the provided value and returns the old value. If this task is a flow rate
 * specifier, no number of repetitions should be provided so -1 is returned.
 */
int Task::setRepetitions(int newRepetitions) {
    if (type == TaskType::FLOWRATE) {
        return -1;
    }

    int old = attr.repetitions;
    attr.repetitions = newRepetitions;
    return old;
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
 * Sets the type of this task to the provided value and returns the old value.
 */
TaskType Task::setType(TaskType newType) {
    TaskType old = type;
    type = newType;
    return old;
}

/*
 * Sets the child pointer to the provided task if this is a cycle node. If not, the member is not set.
 */
void Task::setChild(Task newChild) {
    if (type == TaskType::CYCLE) {
        child = &newChild;
    }
}

/*
 * Returns the target flow rate of this task if it is a flow rate specifier, otherwise returns -1.
 */
float Task::getFlow() {
    if (type != TaskType::FLOWRATE) {
        return -1;
    }

    return attr.flow;
}

/*
 * Returns the repetitions of this task if it is a cycle, otherwise returns -1.
 */
int Task::getRepetitions() {
    if (type != TaskType::CYCLE) {
        return -1;
    }

    return attr.repetitions;
}

/*
 * Returns the value of the duration member of this task.
 */
int Task::getDuration() {
    return duration;
}

/*
 * Returns the type of this task.
 */
TaskType Task::getType() {
    return type;
}

/*
 * Returns a pointer to the child of this task if it is of type cycle.
 */
Task* Task::getChild() {
    if (type != TaskType::CYCLE) {
        return NULL;
    }

    return child;
}
