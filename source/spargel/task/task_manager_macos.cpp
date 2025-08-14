#include "spargel/task/task_manager_macos.h"

namespace spargel::task {
    TaskManager* TaskManager::create() { return new TaskManagerMac; }
}  // namespace spargel::task
