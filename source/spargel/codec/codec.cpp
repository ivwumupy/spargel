#include "spargel/codec/codec.h"

#include "spargel/base/console.h"

namespace spargel::codec {
    void CodecError::dump() {
        auto& console = base::Console::instance();
        for (auto const& msg : messages_) {
            console.write_line(msg.view());
        }
        console.flush();
    }
}  // namespace spargel::codec
