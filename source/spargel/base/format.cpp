#include "spargel/base/format.h"

#include "spargel/base/console.h"

namespace spargel::base {
    namespace detail {
        void ConsoleTarget::append(char const* begin, char const* end) {
            auto& console = Console::instance();
            console.write(begin, end);
        }
    }  // namespace detail
}  // namespace spargel::base
