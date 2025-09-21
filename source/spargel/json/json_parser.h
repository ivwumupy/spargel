#pragma once

#include "spargel/base/either.h"
#include "spargel/base/optional.h"
#include "spargel/base/string.h"
#include "spargel/base/string_view.h"
#include "spargel/json/cursor.h"
#include "spargel/json/json_value.h"

namespace spargel::json {

    // Copied from `codec::CodecError`.
    class JsonParseError {
    public:
        JsonParseError(const base::String& message) : message_(message) {}
        JsonParseError(base::StringView message) : message_(message) {}

        const base::String& message() { return message_; }

        friend JsonParseError operator+(const JsonParseError& error,
                                        const base::StringView& str) {
            return JsonParseError((error.message_ + str).view());
        }

        friend JsonParseError operator+(const JsonParseError& error, char ch) {
            return JsonParseError((error.message_ + ch).view());
        }

        friend JsonParseError operator+(const JsonParseError& error1,
                                        const JsonParseError& error2) {
            return JsonParseError((error1.message_ + error2.message_).view());
        }

    private:
        base::String message_;
    };

    class JsonParser {
    public:
        Cursor cursor;

        void eatWhitespaces();
        base::Either<JsonValue, JsonParseError> parseValue();
        base::Either<JsonObject, JsonParseError> parseObject();
        base::Either<JsonArray, JsonParseError> parseArray();
        base::Either<JsonString, JsonParseError> parseString();
        base::Optional<JsonParseError> parseInteger(JsonNumber& number,
                                                    bool& minus);
        base::Optional<JsonParseError> parseFraction(JsonNumber& number);
        base::Optional<JsonParseError> parseExponent(JsonNumber& number);
        base::Either<JsonNumber, JsonParseError> parseNumber();
        base::Either<JsonBoolean, JsonParseError> parseBoolean();
        base::Optional<JsonParseError> parseNull();
        base::Either<JsonObject, JsonParseError> parseMembers();
        base::Optional<JsonParseError> parseMember(JsonString& key,
                                                   JsonValue& value);
        base::Either<JsonArray, JsonParseError> parseElements();
        base::Either<JsonValue, JsonParseError> parseElement();
    };

    base::Either<JsonValue, JsonParseError> parseJson(const char* str,
                                                      usize length);
    inline base::Either<JsonValue, JsonParseError> parseJson(
        base::StringView s) {
        return parseJson(s.data(), s.length());
    }
}  // namespace spargel::json
