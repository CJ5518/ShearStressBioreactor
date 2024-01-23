#pragma once

#include "task.hpp"
#include "pump.hpp"
#include "flowManager.hpp"

class RoutineManager {
    public:
        void init(FlowManager* fm, Pump* pump, bool test);
        Task* buildTestRoutine();
        void run(Task* head);
        void deleteRoutine(Task* head);

    private:
        FlowManager* f;
        Pump* p;
};