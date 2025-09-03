#include "spargel/json/json_value.h"

namespace spargel::json {

    JsonValue::JsonValue(const JsonValue& other) { construct_from(other); }

    JsonValue::JsonValue(JsonValue&& other) { move_from(base::move(other)); }

    JsonValue& JsonValue::operator=(const JsonValue& other) {
        if (this != &other) {
            destroy();
            construct_from(other);
        }
        return *this;
    }

    JsonValue& JsonValue::operator=(JsonValue&& other) {
        if (this != &other) {
            destroy();
            move_from(base::move(other));
        }
        return *this;
    }

    void JsonValue::destroy() {
        switch (type) {
        case JsonValueType::object:
            object.~JsonObject();
            break;
        case JsonValueType::array:
            array.~JsonArray();
            break;
        case JsonValueType::string:
            string.~JsonString();
            break;
        case JsonValueType::number:
            number.~JsonNumber();
            break;
        case JsonValueType::boolean:
            boolean.~JsonBoolean();
            break;
        case JsonValueType::null:
            break;
        }
        type = JsonValueType::null;
    }

    void JsonValue::construct_from(const JsonValue& other) {
        switch (other.type) {
        case JsonValueType::object:
            base::construct_at(&object, other.object);
            break;
        case JsonValueType::array:
            base::construct_at(&array, other.array);
            break;
        case JsonValueType::string:
            base::construct_at(&string, other.string);
            break;
        case JsonValueType::number:
            base::construct_at(&number, other.number);
            break;
        case JsonValueType::boolean:
            base::construct_at(&boolean, other.boolean);
            break;
        case JsonValueType::null:
            break;
        }
        type = other.type;
    }

    void JsonValue::move_from(JsonValue&& other) {
        switch (other.type) {
        case JsonValueType::object:
            base::construct_at(&object, base::move(other.object));
            break;
        case JsonValueType::array:
            base::construct_at(&array, base::move(other.array));
            break;
        case JsonValueType::string:
            base::construct_at(&string, base::move(other.string));
            break;
        case JsonValueType::number:
            base::construct_at(&number, base::move(other.number));
            break;
        case JsonValueType::boolean:
            base::construct_at(&boolean, base::move(other.boolean));
            break;
        case JsonValueType::null:
            break;
        }
        type = other.type;
    }

}  // namespace spargel::json
