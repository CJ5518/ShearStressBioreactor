/****************************************************************************************************
 * event.hpp
 * Carson Sloan
 * 
 * Declares the Event class and member functions to be used by the GUI for setting task parameters.
/****************************************************************************************************/
#pragma once

#include <Arduino.h>

// Structure for a singly-linked list node that stores task information
class Event {
    public:
        Event(float flow, int duration);
        Event(float flow, int duration, int repetitions);
        Event(float newFlow, int newDuration, int newRepetitions, int newOffDuration);

        float setFlow(float flow);
        int setRepetitions(int repetitions);
        int decRepetitions();
        int setDuration(int duration);
        int setOffDuration(int newDuration);
        Event* setNext(Event* newTask);
        int append(Event* newTask);

        float getFlow();
        int getRepetitions();
        int getDuration();
        int getOffDuration();
        Event* getNext();

    private:
        int duration; // duration to maintain flow in ms
        int offDuration; // duration to maintain 0 ml/min in ms
        float flow; // in ml/min
        int repetitions; // for cycle tasks, 1 otherwise
        Event* next; // pointer to the next task
};