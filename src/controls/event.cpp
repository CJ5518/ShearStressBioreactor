/****************************************************************************************************
 * event.cpp
 * Carson Sloan
 * 
 * Defines the Event class, each object of which represents one required flow rate and its timing
 * information for part of an experiment.
/****************************************************************************************************/

#include "event.hpp"

/*
 * Constructor for a constant flow rate specifier task that sets the target flow rate and duration to the provided values.
 * The number of repetitions is set to 1 by default, and the offDuration is set to 0.
 */
Event::Event(float newFlow, int newDuration) {
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
Event::Event(float newFlow, int newDuration, int newRepetitions) {
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
Event::Event(float newFlow, int newDuration, int newRepetitions, int newOffDuration) {
    flow = newFlow;
    duration = newDuration;
    offDuration = newOffDuration;
    repetitions = newRepetitions;
    next = NULL;
}

/*
 * Sets the flow rate member to the provided value and returns the old value.
 */
float Event::setFlow(float newFlow) {
    int old = flow;
    flow = newFlow;
    return old;
}

/*
 * Sets the repetitions member to the provided value and returns the old value.
 */
int Event::setRepetitions(int newRepetitions) {
    int old = repetitions;
    repetitions = newRepetitions;
    return old;
}

/*
 * Decrements the repetitions member by 1 and returns the old value.
 */
int Event::decRepetitions() {
    return repetitions--;
}

/*
 * Sets the duration of this task to the provided value and returns the old value.
 */
int Event::setDuration(int newDuration) {
    int old = duration;
    duration = newDuration;
    return old;
}

/*
 * Sets the off duration of this task to the provided value and returns the old value.
 */
int Event::setOffDuration(int newDuration) {
    int old = offDuration;
    offDuration = newDuration;
    return old;
}

/*
 * Sets the provided task as the next task following this one, and returns the old value of the next pointer.
 */
Event* Event::setNext(Event* newTask) {
    Event* oldTask = next;
    next = newTask;
    
    return oldTask;
}

/*
 * Appends the provided task to the end of the linked list, and returns the number of tasks following this one in the new list.
 */
int Event::append(Event* newTask) {
    int i = 1; // account for the new task
    Event* head = this;

    // Loop until there is no next task
    while (head->next != NULL) {
        i++;
        head = head->next;
    }

    head->next = newTask;
    return i;
}

/*
 * Returns the target flow rate of this task.
 */
float Event::getFlow() {
    return flow;
}

/*
 * Returns the repetitions of this task if it is a cycle.
 */
int Event::getRepetitions() {
    return repetitions;
}

/*
 * Returns the value of the duration member of this task.
 */
int Event::getDuration() {
    return duration;
}

/*
 * Returns the duration this task will stay off.
 */
int Event::getOffDuration() {
    return offDuration;
}

/*
 * Returns a pointer to the next task in the linked list.
 */
Event* Event::getNext() {
    return next;
}