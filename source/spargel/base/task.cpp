#include "spargel/base/task.h"

namespace spargel::base {
Task Task::PromiseType::get_return_object() {
  return Task{HandleType::from_promise(*this)};
}
}
