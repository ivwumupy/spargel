#pragma once

#include "spargel/task/task_manager.h"

namespace spargel::task {
    class TaskManagerMac final : public TaskManager {
    public:
        void postTask(Task*) override {}
    };
}  // namespace spargel::task
