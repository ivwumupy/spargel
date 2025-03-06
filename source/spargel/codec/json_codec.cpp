#include <spargel/codec/codec.h>
#include <spargel/codec/json.h>

namespace spargel::codec {

    base::Either<bool, JsonDecodeError> DecodeBackendJson::getBoolean(const JsonValue& data) {
        if (data.type == JsonValueType::boolean)
            return base::makeLeft<bool, JsonDecodeError>(data.boolean);
        else
            return base::makeRight<bool, JsonDecodeError>("expected a boolean");
    }

    base::Either<u32, JsonDecodeError> DecodeBackendJson::getU32(const JsonValue& data) {
        if (data.type == JsonValueType::number) {
            u32 n = u32(data.number);
            if (JsonNumber(n) != data.number)
                return base::makeRight<u32, JsonDecodeError>("expected number to be U32");
            else
                return base::makeLeft<u32, JsonDecodeError>(data.number);
        } else {
            return base::makeRight<u32, JsonDecodeError>("expected a number");
        }
    }

    base::Either<i32, JsonDecodeError> DecodeBackendJson::getI32(const JsonValue& data) {
        if (data.type == JsonValueType::number) {
            i32 n = i32(data.number);
            if (JsonNumber(n) != data.number)
                return base::makeRight<i32, JsonDecodeError>("expected number to be I32");
            else
                return base::makeLeft<i32, JsonDecodeError>(data.number);
        } else {
            return base::makeRight<i32, JsonDecodeError>("expected a number");
        }
    }

    base::Either<base::string, JsonDecodeError> DecodeBackendJson::getString(
        const JsonValue& data) {
        if (data.type == JsonValueType::string)
            return base::makeLeft<base::string, JsonDecodeError>(data.string);
        else
            return base::makeRight<base::string, JsonDecodeError>("expected a string");
    }

    base::Either<base::vector<JsonValue>, JsonDecodeError> DecodeBackendJson::getArray(
        const JsonValue& data) {
        if (data.type == JsonValueType::array)
            return base::makeLeft<base::vector<JsonValue>, JsonDecodeError>(data.array.elements);
        else
            return base::makeRight<base::vector<JsonValue>, JsonDecodeError>("expected an array");
    }

    base::Either<base::Optional<JsonValue>, JsonDecodeError> DecodeBackendJson::getMember(
        const JsonValue& data, const base::string& key) {
        if (data.type == JsonValueType::object) {
            auto* ptr = data.object.members.get(key);
            auto optional = base::Optional<JsonValue>();
            if (ptr != nullptr) optional = base::makeOptional<JsonValue>(*ptr);
            return base::makeLeft<base::Optional<JsonValue>, JsonDecodeError>(base::move(optional));
        } else {
            return base::makeRight<base::Optional<JsonValue>, JsonDecodeError>(
                "expected an object");
        }
    }

}  // namespace spargel::codec
