#pragma once

#include <spargel/base/hash_map.h>
#include <spargel/base/optional.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>
#include <spargel/codec/codec.h>

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

    // TODO: change to SumType?
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

    class JsonParseError : public CodecError {};

    /*
     * Parser
     */

    base::Either<JsonValue, JsonParseError> parseJson(const char* str, usize length,
                                                      JsonValue& value);

    /*
     * Utils
     */

    base::Either<const JsonObject*, JsonParseError> getJsonMemberObject(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional = false);

    base::Either<const JsonArray*, JsonParseError> getJsonMemberArray(const JsonObject& json,
                                                                      const JsonString& key,
                                                                      bool optional = false);

    base::Either<const JsonString*, JsonParseError> getJsonMemberString(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional = false);

    base::Either<const JsonNumber*, JsonParseError> getJsonMemberNumber(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional = false);

}  // namespace spargel::codec
