#include "spargel/base/trace.h"

namespace spargel::base {

    TraceEngine* TraceEngine::getInstance() {
        static TraceEngine inst;
        return &inst;
    }

}  // namespace spargel::base
