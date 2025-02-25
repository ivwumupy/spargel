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

    struct JsonValue;

    struct JsonObject {
        base::HashMap<JsonString, JsonValue> members;

        JsonObject() : members(base::default_allocator()) {}
    };

    struct JsonArray {
        base::vector<JsonValue> elements;
    };

    struct JsonValue {
        JsonValueType type;
        union {
            JsonObject object;
            JsonArray array;
            JsonString string;
            JsonNumber number;
            JsonBoolean boolean;
        };

        JsonValue() : type(JsonValueType::null) {}

        JsonValue(const JsonValue& other);

        ~JsonValue();
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

        base::string const& getMessage() const { return _msg; }

    private:
        JsonParseResult(bool failed, const base::string& msg) : _failed(failed), _msg(msg) {}

        bool _failed;
        base::string _msg;
    };

    JsonParseResult parseJson(const char* str, usize length, JsonValue& value);

}  // namespace spargel::codec
