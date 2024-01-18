#pragma once

#include <Arduino.h>

// Structure for a singly-linked list node that stores task information
class Task {
    public:
        Task(float flow, int duration);
        Task(float flow, int duration, int repetitions);
        Task(float newFlow, int newDuration, int newRepetitions, int newOffDuration);

        float setFlow(float flow);
        int setRepetitions(int repetitions);
        int decRepetitions();
        int setDuration(int duration);
        int setOffDuration(int newDuration);
        Task* setNext(Task* newTask);
        int append(Task* newTask);

        float getFlow();
        int getRepetitions();
        int getDuration();
        int getOffDuration();
        Task* getNext();

    private:
        int duration; // duration to maintain flow in ms
        int offDuration; // duration to maintain 0 ml/min in ms
        float flow; // in ml/min
        int repetitions; // for cycle tasks, 1 otherwise
        Task* next; // pointer to the next task
};