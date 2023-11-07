#pragma once

#include <Arduino.h>

enum class TaskType {CYCLE, FLOWRATE}; // possible types of tasks

// Structure for a singly-linked list node that stores task information
class Task {
    public:
        Task* next; // pointer to the next task
        
        Task(float flow, int duration);
        Task(Task child, int repetitions);

        float setFlow(float flow);
        int setRepetitions(int repetitions);
        int setDuration(int duration);
        TaskType setType(TaskType type);
        void setChild(Task newChild);

        float getFlow();
        int getRepetitions();
        int getDuration();
        TaskType getType();
        Task* getChild();

    private:
        TaskType type; // cycle or flow rate specifier
        int duration; // duration to run in ms
        union {
            float flow; // for FLOWRATE tasks, in ml/min
            int repetitions; // for CYCLE tasks
        } attr;

        Task* child; // only set for cycle tasks, these are the tasks to repeat
};