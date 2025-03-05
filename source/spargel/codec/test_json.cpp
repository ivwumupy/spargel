#include <spargel/codec/test_json.h>

namespace {

    void test_JsonValue() {
        JsonValue v;
        v = JsonValue(JsonObject());
        v.object.members.set(JsonString("error"), JsonValue());
        v.object.members.set(JsonString("active"), JsonBoolean());

        spargel_assert(v.type == JsonValueType::object);
        spargel_assert(v.object.members.count() == 2);

        JsonValue v2 = v;
        v2 = JsonValue(JsonArray());
        v2.array.elements.push(JsonNumber(123.456));
        v2.array.elements.push(JsonString("hello"));

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
        auto result = base::makeLeft<JsonValue, JsonParseError>();

        result = parseJson("null");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonValue()));

        result = parseJson("true");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

        result = parseJson("false");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

        result = parseJson("\"string\"");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("string")));

        result = parseJson("\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"");
        spargel_assert(result.isLeft() &&
                       isEqual(result.left(), JsonString(" \" \\ / \b \f \n \r \t ")));

        result = parseJson("\"\\u0020\\u0041\\u0061\"");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonString(" Aa")));

        result = parseJson("\"\\u4e2d\\u6587\"");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("中文")));

        result = parseJson("\"\\u65e5\\u672c\\u8a9e\"");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("日本語")));

        result = parseJson("\"\\ud55c\\uad6d\\uc5b4\"");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("한국어")));

        result = parseJson("\"\n\"");
        spargel_assert(result.isRight());

        result = parseJson("0");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(0)));

        result = parseJson("12345");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(12345)));

        result = parseJson("-54321");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-54321)));

        result = parseJson("123.000456");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(123.000456)));

        result = parseJson("-654.000321");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-654.000321)));

        result = parseJson("0.001");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(0.001)));

        result = parseJson("-0.001");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-0.001)));

        result = parseJson("0.001e4");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(0.001e4)));

        result = parseJson("-0.001e+3");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-0.001e+3)));

        result = parseJson("-0.001E-3");
        spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-0.001E-3)));
    }

    void test_array() {
        auto result = base::makeLeft<JsonValue, JsonParseError>();

        result = parseJson("[]");
        spargel_assert(result.isLeft() && result.left().type == JsonValueType::array);
        spargel_assert(result.left().array.elements.count() == 0);

        result = parseJson("[ true, \"ABC\", -123.456, null, false ]");
        spargel_assert(result.isLeft() && result.left().type == JsonValueType::array);
        auto array = result.left().array;
        spargel_assert(array.elements.count() == 5);
        spargel_assert(isEqual(array.elements[0], JsonBoolean(true)));
        spargel_assert(isEqual(array.elements[1], JsonString("ABC")));
        spargel_assert(isEqual(array.elements[2], JsonNumber(-123.456)));
        spargel_assert(isEqual(array.elements[3], JsonValue()));
        spargel_assert(isEqual(array.elements[4], JsonBoolean(false)));
    }

    void test_object() {
        auto result = base::makeLeft<JsonValue, JsonParseError>();

        result = parseJson("{}");
        spargel_assert(result.isLeft() && result.left().type == JsonValueType::object);
        spargel_assert(result.left().object.members.count() == 0);

        result =
            parseJson("{ \"error\": null, \"name\": \"Alice\", \"age\": 20, \"happy\": true }");

        spargel_assert(result.isLeft() && result.left().type == JsonValueType::object);
        auto object = result.left().object;
        spargel_assert(object.members.count() == 4);
        spargel_assert(isMemberEqual(object, JsonString("error"), JsonValue()));
        spargel_assert(isMemberEqual(object, JsonString("name"), JsonString("Alice")));
        spargel_assert(isMemberEqual(object, JsonString("age"), JsonNumber(20)));
        spargel_assert(isMemberEqual(object, JsonString("happy"), JsonBoolean(true)));
    }

    void test_composite() {
        auto result = base::makeLeft<JsonValue, JsonParseError>();
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
        result = parseJson(str);
        spargel_assert(result.isLeft());
        auto value = result.left();

        spargel_assert(value.type == JsonValueType::object);
        spargel_assert(value.object.members.count() == 3);

        spargel_assert(
            isMemberEqual(value.object, JsonString("model"), JsonString("deepseek-chat")));

        auto* ptr1 = value.object.members.get(JsonString("choices"));
        spargel_assert(ptr1 != nullptr);
        auto& v1 = *ptr1;
        spargel_assert(v1.type == JsonValueType::array);
        spargel_assert(v1.array.elements.count() == 1);
        auto& v11 = v1.array.elements[0];
        spargel_assert(v11.type == JsonValueType::object);
        spargel_assert(isMemberEqual(v11.object, JsonString("index"), JsonNumber(0)));
        spargel_assert(isMemberEqual(v11.object, JsonString("role"), JsonString("assistant")));
        spargel_assert(isMemberEqual(v11.object, JsonString("logprobs"), JsonValue()));
        spargel_assert(isMemberEqual(v11.object, JsonString("finish_reason"), JsonString("stop")));

        auto* ptr2 = value.object.members.get(JsonString("usage"));
        spargel_assert(ptr2 != nullptr);
        auto& v2 = *ptr2;
        spargel_assert(v2.type == JsonValueType::object);
        spargel_assert(isMemberEqual(v2.object, JsonString("prompt_tokens"), JsonNumber(11)));
        spargel_assert(isMemberEqual(v2.object, JsonString("completion_tokens"), JsonNumber(11)));
        spargel_assert(isMemberEqual(v2.object, JsonString("total_tokens"), JsonNumber(22)));
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
