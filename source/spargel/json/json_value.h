#pragma once

#include "spargel/base/hash_map.h"
#include "spargel/base/string.h"
#include "spargel/base/vector.h"

namespace spargel::json {

    enum class JsonValueType {
        object,
        array,
        string,
        number,
        boolean,
        null,
    };

    struct JsonNull {};

    using JsonString = base::String;

    using JsonNumber = double;

    using JsonBoolean = bool;

    class JsonValue;

    struct JsonObject {
        base::HashMap<JsonString, JsonValue> members;

        JsonObject() {}

        JsonObject(const base::HashMap<JsonString, JsonValue>& members) : members(members) {}
        JsonObject(base::HashMap<JsonString, JsonValue>&& members) : members(base::move(members)) {}
    };

    struct JsonArray {
        base::vector<JsonValue> elements;
    };

    // TODO: change to tagged union?
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

        JsonValue(JsonNull) : type(JsonValueType::null) {}
        JsonValue(JsonObject&& object) : type(JsonValueType::object), object(base::move(object)) {}
        JsonValue(JsonArray&& array) : type(JsonValueType::array), array(base::move(array)) {}
        JsonValue(JsonString&& string) : type(JsonValueType::string), string(base::move(string)) {}
        JsonValue(JsonNumber&& number) : type(JsonValueType::number), number(base::move(number)) {}
        JsonValue(JsonBoolean&& boolean)
            : type(JsonValueType::boolean), boolean(base::move(boolean)) {}

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


}  // namespace spargel::json
