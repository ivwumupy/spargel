#include <spargel/base/assert.h>
#include <spargel/codec/json.h>

// libc
#include <string.h>

using namespace spargel;

namespace {

    codec::JsonParseResult parseJson(const char* str, codec::JsonValue& value) {
        return parseJson(str, strlen(str), value);
    }

}

int main() {
    codec::JsonValue value;
    codec::JsonParseResult result;

    result = parseJson("null", value);
    spargel_assert(!result.failed());
    spargel_assert(value.type == codec::JsonValueType::null);

    result = parseJson("true", value);
    spargel_assert(!result.failed());
    spargel_assert(value.type == codec::JsonValueType::boolean);
    spargel_assert(value.boolean == true);

    result = parseJson("false", value);
    spargel_assert(!result.failed());
    spargel_assert(value.type == codec::JsonValueType::boolean);
    spargel_assert(value.boolean == false);

    result = parseJson("\"string\"", value);
    spargel_assert(!result.failed());
    spargel_assert(value.type == codec::JsonValueType::string);
    spargel_assert(value.string == base::string("string"));

    return 0;
}
