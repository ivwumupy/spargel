#include <spargel/base/assert.h>
#include <spargel/codec/json.h>

// libc
#include <string.h>

using namespace spargel;
using namespace spargel::codec;

namespace {

    JsonParseResult parseJson(const char* str, JsonValue& value) {
        return parseJson(str, strlen(str), value);
    }

    bool isEqual(const JsonValue& v1, const JsonValue& v2) {
        if (&v1 == &v2) return true;
        if (v1.type != v2.type) return false;

        switch (v1.type) {
        case JsonValueType::string:
            return v1.string == v2.string;
        case JsonValueType::number:
            return v1.number == v2.number;
        case JsonValueType::boolean:
            return v1.boolean == v2.boolean;
        case JsonValueType::null:
            return true;
        default:
            return false;
        }
    }

    void test_JsonValue() {
        JsonValue v;
        v = JsonValue(JsonObject());
        v.object.members.set(JsonString("error"), JsonValue());
        v.object.members.set(JsonString("active"), JsonValue(JsonBoolean()));

        spargel_assert(v.type == JsonValueType::object);
        spargel_assert(v.object.members.count() == 2);

        JsonValue v2 = v;
        v2 = JsonValue(JsonArray());
        v2.array.elements.push(JsonValue(JsonNumber(123.456)));
        v2.array.elements.push(JsonValue(JsonString("hello")));

        spargel_assert(v2.type == JsonValueType::array);
        spargel_assert(v2.array.elements.count() == 2);

        spargel_assert(v.type == JsonValueType::object);
        spargel_assert(v.object.members.count() == 2);

        v = base::move(v2);
        spargel_assert(v.type == JsonValueType::array);
        spargel_assert(v.array.elements.count() == 2);

        JsonValue v3 = base::move(v);
        spargel_assert(v3.type == JsonValueType::array);
        spargel_assert(v3.array.elements.count() == 2);
    }

    void test_primitive() {
        JsonValue value;
        JsonParseResult result;

        result = parseJson("null", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::null);

        result = parseJson("true", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::boolean);
        spargel_assert(value.boolean == true);

        result = parseJson("false", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::boolean);
        spargel_assert(value.boolean == false);

        result = parseJson("\"string\"", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::string);
        spargel_assert(value.string == base::string("string"));
    }

    void test_array() {
        JsonValue value;
        JsonParseResult result;

        result = parseJson("[]", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::array);
        spargel_assert(value.array.elements.count() == 0);

        result = parseJson("[ true, \"ABC\", null, false ]", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::array);
        spargel_assert(value.array.elements.count() == 4);
        spargel_assert(isEqual(value.array.elements[0], JsonValue(JsonBoolean(true))));
        spargel_assert(isEqual(value.array.elements[1], JsonValue(JsonString("ABC"))));
        spargel_assert(isEqual(value.array.elements[2], JsonValue()));
        spargel_assert(isEqual(value.array.elements[3], JsonValue(JsonBoolean(false))));
    }

    void test_object() {
        JsonValue value;
        JsonParseResult result;

        result = parseJson("{}", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 0);

        result = parseJson("{ \"error\": null, \"name\": \"Alice\", \"good\": true }", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 3);
        spargel_assert(value.object.members.get(JsonString("error")) != nullptr);
        spargel_assert(isEqual(*value.object.members.get(JsonString("error")), JsonValue()));
        spargel_assert(value.object.members.get(JsonString("name")) != nullptr);
        spargel_assert(
            isEqual(*value.object.members.get(JsonString("name")), JsonValue(JsonString("Alice"))));
        spargel_assert(value.object.members.get(JsonString("good")) != nullptr);
        spargel_assert(
            isEqual(*value.object.members.get(JsonString("good")), JsonValue(JsonBoolean(true))));
    }

    void test_composite() {}

}  // namespace

int main() {
    test_JsonValue();

    test_primitive();

    test_array();

    test_object();

    test_composite();

    return 0;
}
