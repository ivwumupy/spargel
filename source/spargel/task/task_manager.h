#pragma once

#include "spargel/base/meta.h"

namespace spargel::task {
    class Task {
    public:
        virtual ~Task() = default;
        virtual void execute() = 0;
    };
    namespace detail {
        template <typename F>
        struct TaskAdapter final : public Task {
            void execute() override { func(); }
            F func;
        };
    }  // namespace detail
    class TaskManager {
    public:
        static TaskManager* create();

        virtual ~TaskManager() = default;

        // Ownership is transferred to the TaskManager.
        virtual void postTask(Task* task) = 0;

        template <typename F>
        void postTask(F&& f) {
            postTask(new detail::TaskAdapter<F>{base::forward<F>(f)});
        }
    };
}  // namespace spargel::task
