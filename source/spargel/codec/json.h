#pragma once

#include <spargel/base/hash_map.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>

namespace spargel::codec {

    enum class JsonValueType {
        object,
        array,
        string,
        number,
        boolean,
        null,
    };

    using JsonString = base::string;

    using JsonNumber = double;

    using JsonBoolean = bool;

    class JsonValue;

    struct JsonObject {
        base::HashMap<JsonString, JsonValue> members;

        JsonObject() : members(base::default_allocator()) {}
    };

    struct JsonArray {
        base::vector<JsonValue> elements;
    };

    class JsonValue {
    public:
        JsonValueType type;
        union {
            JsonObject object;
            JsonArray array;
            JsonString string;
            JsonNumber number;
            JsonBoolean boolean;
        };

        JsonValue() : type(JsonValueType::null) {}

        explicit JsonValue(const JsonObject& object)
            : type(JsonValueType::object), object(object) {}

        explicit JsonValue(const JsonArray& array) : type(JsonValueType::array), array(array) {}

        explicit JsonValue(const JsonString& string)
            : type(JsonValueType::string), string(string) {}

        explicit JsonValue(JsonNumber number) : type(JsonValueType::number), number(number) {}

        explicit JsonValue(JsonBoolean boolean) : type(JsonValueType::boolean), boolean(boolean) {}

        JsonValue(const JsonValue& other);

        JsonValue(JsonValue&& other);

        JsonValue& operator=(const JsonValue& other);

        JsonValue& operator=(JsonValue&& other);

        ~JsonValue() { destroy(); }

    private:
        void destroy();

        void construct_from(const JsonValue& other);

        void move_from(JsonValue&& other);
    };

    class JsonParseResult {
    public:
        JsonParseResult() : _failed(false), _msg("") {}

        bool failed() const { return _failed; }

        static JsonParseResult success() { return JsonParseResult(false, base::string()); }

        static JsonParseResult error(const base::string& msg) { return JsonParseResult(true, msg); }

        static JsonParseResult error(const char* msg = "failed to parse JSON") {
            return error(base::string(msg));
        }

        base::string const& message() const { return _msg; }

    private:
        JsonParseResult(bool failed, const base::string& msg) : _failed(failed), _msg(msg) {}

        bool _failed;
        base::string _msg;
    };

    JsonParseResult parseJson(const char* str, usize length, JsonValue& value);

}  // namespace spargel::codec
