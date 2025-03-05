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

    base::Either<JsonValue, JsonParseError> parseJson(const char* str, usize length);

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

    /*
     * Codec Backend
     */

    class JsonEncodeError : public CodecError {};
    class JsonDecodeError : public CodecError {};

    // JSON encode backend
    struct EncodeBackedJson {
        using DataType = JsonValue;
        using ErrorType = JsonEncodeError;

        base::Either<JsonValue, JsonEncodeError> makeBoolean(bool b) {
            return base::makeLeft<JsonBoolean, JsonEncodeError>(b);
        }

        base::Either<JsonValue, JsonEncodeError> makeU32(u32 n) {
            return base::makeLeft<JsonNumber, JsonEncodeError>(n);
        }
        base::Either<JsonValue, JsonEncodeError> makeI32(i32 n) {
            return base::makeLeft<JsonNumber, JsonEncodeError>(n);
        }

        base::Either<JsonValue, JsonEncodeError> makeString(const base::string& s) {
            return base::makeLeft<JsonString, JsonEncodeError>(s);
        }

        base::Either<JsonValue, JsonEncodeError> makeArray(const base::vector<JsonValue>& array) {
            return base::makeLeft<JsonArray, JsonEncodeError>(array);
        }
    };

    // JSON decode backend
    struct DecodeBackendJson {
        using DataType = JsonValue;
        using ErrorType = JsonDecodeError;

        base::Either<bool, JsonDecodeError> getBoolean(const JsonValue& data);

        base::Either<u32, JsonDecodeError> getU32(const JsonValue& data);
        base::Either<i32, JsonDecodeError> getI32(const JsonValue& data);

        base::Either<base::string, JsonDecodeError> getString(const JsonValue& data);

        base::Either<base::vector<JsonValue>, JsonDecodeError> getArray(const JsonValue& data);
    };

}  // namespace spargel::codec
