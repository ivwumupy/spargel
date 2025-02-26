#include <spargel/base/assert.h>
#include <spargel/codec/json.h>

// libc
#include <string.h>

// libm
#include <math.h>

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
            return abs(v1.number - v2.number) < 1e-9;
        case JsonValueType::boolean:
            return v1.boolean == v2.boolean;
        case JsonValueType::null:
            return true;
        default:
            return false;
        }
    }

    bool isMemberEqual(JsonObject& object, const JsonString& key, const JsonValue& v) {
        auto* ptr = object.members.get(key);
        if (ptr == nullptr) return false;

        return isEqual(*ptr, v);
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
        spargel_assert(isEqual(value, JsonValue()));

        result = parseJson("true", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonBoolean(true))));

        result = parseJson("false", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonBoolean(false))));

        result = parseJson("\"string\"", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonString("string"))));

        result = parseJson("\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonString(" \" \\ / \b \f \n \r \t "))));

        result = parseJson("\"\u0020\u0041\u0061\"", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonString(" Aa"))));

        result = parseJson("\"\n\"", value);
        spargel_assert(result.failed());

        result = parseJson("0", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(0))));

        result = parseJson("12345", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(12345))));

        result = parseJson("-54321", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(-54321))));

        result = parseJson("123.000456", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(123.000456))));

        result = parseJson("-654.000321", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(-654.000321))));

        result = parseJson("0.001", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(0.001))));

        result = parseJson("-0.001", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(-0.001))));

        result = parseJson("0.001e4", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(0.001e4))));

        result = parseJson("-0.001e+3", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(-0.001e+3))));

        result = parseJson("-0.001E-3", value);
        spargel_assert(!result.failed());
        spargel_assert(isEqual(value, JsonValue(JsonNumber(-0.001E-3))));
    }

    void test_array() {
        JsonValue value;
        JsonParseResult result;

        result = parseJson("[]", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::array);
        spargel_assert(value.array.elements.count() == 0);

        result = parseJson("[ true, \"ABC\", -123.456, null, false ]", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::array);
        spargel_assert(value.array.elements.count() == 5);
        spargel_assert(isEqual(value.array.elements[0], JsonValue(JsonBoolean(true))));
        spargel_assert(isEqual(value.array.elements[1], JsonValue(JsonString("ABC"))));
        spargel_assert(isEqual(value.array.elements[2], JsonValue(JsonNumber(-123.456))));
        spargel_assert(isEqual(value.array.elements[3], JsonValue()));
        spargel_assert(isEqual(value.array.elements[4], JsonValue(JsonBoolean(false))));
    }

    void test_object() {
        JsonValue value;
        JsonParseResult result;

        result = parseJson("{}", value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 0);

        result = parseJson("{ \"error\": null, \"name\": \"Alice\", \"age\": 20, \"happy\": true }",
                           value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 4);
        spargel_assert(isMemberEqual(value.object, JsonString("error"), JsonValue()));
        spargel_assert(
            isMemberEqual(value.object, JsonString("name"), JsonValue(JsonString("Alice"))));
        spargel_assert(isMemberEqual(value.object, JsonString("age"), JsonValue(JsonNumber(20))));
        spargel_assert(
            isMemberEqual(value.object, JsonString("happy"), JsonValue(JsonBoolean(true))));
    }

    void test_composite() {
        JsonValue value;
        JsonParseResult result;
        const char* str;

        str =
            "{\n"
            "   \"model\": \"deepseek-chat\",\n"
            "   \"choices\": [\n"
            "       {\n"
            "           \"index\": 0,\n"
            "           \"role\": \"assistant\",\n"
            "           \"logprobs\":null,\n"
            "           \"finish_reason\":\"stop\"\n"
            "       }\n"
            "   ],\n"
            "   \"usage\": {\n"
            "       \"prompt_tokens\":11,\n"
            "       \"completion_tokens\":11,\n"
            "       \"total_tokens\":22\n"
            "   }\n"
            "}";
        result = parseJson(str, value);
        spargel_assert(!result.failed());
        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 3);

        spargel_assert(isMemberEqual(value.object, JsonString("model"),
                                     JsonValue(JsonString("deepseek-chat"))));

        auto* ptr1 = value.object.members.get(JsonString("choices"));
        spargel_assert(ptr1 != nullptr);
        auto& v1 = *ptr1;
        spargel_assert(v1.type == JsonValueType::array);
        spargel_assert(v1.array.elements.count() == 1);
        auto& v11 = v1.array.elements[0];
        spargel_assert(v11.type == JsonValueType::object);
        spargel_assert(isMemberEqual(v11.object, JsonString("index"), JsonValue(JsonNumber(0))));
        spargel_assert(
            isMemberEqual(v11.object, JsonString("role"), JsonValue(JsonString("assistant"))));
        spargel_assert(isMemberEqual(v11.object, JsonString("logprobs"), JsonValue()));
        spargel_assert(
            isMemberEqual(v11.object, JsonString("finish_reason"), JsonValue(JsonString("stop"))));

        auto* ptr2 = value.object.members.get(JsonString("usage"));
        spargel_assert(ptr2 != nullptr);
        auto& v2 = *ptr2;
        spargel_assert(v2.type == JsonValueType::object);
        spargel_assert(
            isMemberEqual(v2.object, JsonString("prompt_tokens"), JsonValue(JsonNumber(11))));
        spargel_assert(
            isMemberEqual(v2.object, JsonString("completion_tokens"), JsonValue(JsonNumber(11))));
        spargel_assert(
            isMemberEqual(v2.object, JsonString("total_tokens"), JsonValue(JsonNumber(22))));
    }

}  // namespace

int main() {
    test_JsonValue();

    test_primitive();

    test_array();

    test_object();

    test_composite();

    return 0;
}
