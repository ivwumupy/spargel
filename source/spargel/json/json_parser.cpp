#include "spargel/base/either.h"
#include "spargel/base/optional.h"
#include "spargel/base/string_view.h"
#include "spargel/base/trace.h"
#include "spargel/json/cursor.h"
#include "spargel/json/json.h"

// libm
#include <math.h>

namespace spargel::json {

    using namespace base::literals;

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

    namespace {
        using base::String;

        using base::Either;
        using base::Left;
        using base::Right;

        using base::makeOptional;
        using base::nullopt;
        using base::Optional;

        const auto UNEXPECTED_END = JsonParseError("unexpected end"_sv);

        String char2hex(char ch) {
            const char hexDigits[] = "0123456789abcdef";
            return String(hexDigits[(ch >> 4) & 0xf]) + hexDigits[ch & 0xf];
        }

        void appendUtf8(base::vector<char>& chars, u32 code) {
            if (code <= 0x7f) {
                chars.emplace((char)code);
            } else if (code <= 0x7ff) {
                chars.emplace((char)(0xc0 | (code >> 6)));
                chars.emplace((char)(0x80 | (code & 0x3f)));
            } else if (code <= 0xffff) {
                chars.emplace((char)(0xe0 | (code >> 12)));
                chars.emplace((char)(0x80 | ((code >> 6) & 0x3f)));
                chars.emplace((char)(0x80 | (code & 0x3f)));
            } else if (code <= 0x10ffff) {
                chars.emplace((char)(0xf0 | (code >> 18)));
                chars.emplace((char)(0x80 | ((code >> 12) & 0x3f)));
                chars.emplace((char)(0x80 | ((code >> 6) & 0x3f)));
                chars.emplace((char)(0x80 | (code & 0x3f)));
            }
        }

        class JsonParser {
        public:
            struct JsonParseContext {
                Cursor cursor;
            };
            void eatWhitespaces(JsonParseContext& ctx);
            Either<JsonValue, JsonParseError> parseValue(JsonParseContext& ctx);
            Either<JsonObject, JsonParseError> parseObject(JsonParseContext& ctx);
            Either<JsonArray, JsonParseError> parseArray(JsonParseContext& ctx);
            Either<JsonString, JsonParseError> parseString(JsonParseContext& ctx);
            Optional<JsonParseError> parseInteger(JsonParseContext& ctx, JsonNumber& number,
                                                  bool& minus);
            Optional<JsonParseError> parseFraction(JsonParseContext& ctx, JsonNumber& number);
            Optional<JsonParseError> parseExponent(JsonParseContext& ctx, JsonNumber& number);
            Either<JsonNumber, JsonParseError> parseNumber(JsonParseContext& ctx);
            Either<JsonBoolean, JsonParseError> parseBoolean(JsonParseContext& ctx);
            Optional<JsonParseError> parseNull(JsonParseContext& ctx);
            Either<JsonObject, JsonParseError> parseMembers(JsonParseContext& ctx);
            Optional<JsonParseError> parseMember(JsonParseContext& ctx, JsonString& key,
                                                 JsonValue& value);
            Either<JsonArray, JsonParseError> parseElements(JsonParseContext& ctx);
            Either<JsonValue, JsonParseError> parseElement(JsonParseContext& ctx);
        };

        /*
         * The syntax descriptions in the following comments are from:
         *   https://www.json.org/json-en.html
         * The actual syntax we are using is slightly different from it.
         */

        /*
         * ws:
         *   ""
         *   '0020' ws
         *   '000A' ws
         *   '000D' ws
         *   '0009' ws
         */
        void JsonParser::eatWhitespaces(JsonParseContext& ctx) {
            spargel_trace_scope("eatWhitespaces");

            auto& cursor = ctx.cursor;
            while (!cursorIsEnd(cursor)) {
                char ch = (char)cursorPeek(cursor);
                if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
                    cursorAdvance(cursor);
                else
                    break;
            }
        }

        /*
         * value:
         *   object
         *   array
         *   string
         *   number
         *   "true"
         *   "false"
         *   "null"
         */
        Either<JsonValue, JsonParseError> JsonParser::parseValue(JsonParseContext& ctx) {
            spargel_trace_scope("parseValue");

            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return Right(JsonParseError("expected a value"_sv));

            char ch = (char)cursorPeek(cursor);
            switch (ch) {
            case '{':
                /* object */
                return parseObject(ctx);
            case '[':
                /* array */
                return parseArray(ctx);
            case '"':
                /* string */
                return parseString(ctx);
            case 't':
            case 'f':
                /* boolean: true/false */
                return parseBoolean(ctx);
            case 'n': {
                /* null */
                auto result = parseNull(ctx);
                if (result.hasValue()) {
                    return Right(base::move(result.value()));
                } else {
                    return Left(JsonValue());
                }
            }
            default:
                /* number */
                if ((ch >= '0' && ch <= '9') || ch == '-') {
                    return parseNumber(ctx);
                } else {
                    return Right(JsonParseError(String("unexpected character: '") + ch + '\''));
                }
            }
        }

        /*
         * object:
         *   '{' ws '}'
         *   '{' members '}'
         */
        Either<JsonObject, JsonParseError> JsonParser::parseObject(JsonParseContext& ctx) {
            spargel_trace_scope("parseObject");

            auto& cursor = ctx.cursor;

            // '{' ws
            if (!cursorTryEatChar(cursor, '{')) return Right(JsonParseError("expected a value"_sv));
            eatWhitespaces(ctx);
            if (cursorIsEnd(cursor)) return Right(JsonParseError(UNEXPECTED_END));

            // '}'
            if (cursorTryEatChar(cursor, '}')) return Left(JsonObject());

            // members '}'
            auto result = parseMembers(ctx);
            if (result.isRight()) return result;
            if (!cursorTryEatChar(cursor, '}')) return Right(JsonParseError("expected '}'"_sv));

            return result;
        }

        /*
         * array:
         *   '[' ws ']'
         *   '[' elements ']'
         */
        Either<JsonArray, JsonParseError> JsonParser::parseArray(JsonParseContext& ctx) {
            spargel_trace_scope("parseArray");

            auto& cursor = ctx.cursor;

            // '[' ws
            if (!cursorTryEatChar(cursor, '[')) return Right(JsonParseError("expected '['"_sv));
            eatWhitespaces(ctx);
            if (cursorIsEnd(cursor)) return Right(JsonParseError(UNEXPECTED_END));

            // ']'
            if (cursorTryEatChar(cursor, ']')) return Left(JsonArray());

            // elements ']'
            auto result = parseElements(ctx);
            if (result.isRight()) return result;
            if (!cursorTryEatChar(cursor, ']')) return Right(JsonParseError("expected ']'"_sv));

            return result;
        }

        /*
        bool isGood(char c, char d) {
            return (c != '"' && d != '"') && (c != '\\' && d != '\\') && (c >= 0x20 && d >= 0x20);
        }
        */

        bool isGood(char c) { return c != '"' && c != '\\' && c >= 0x20; }

        /*
         * string:
         *   '"' characters '"'
         * character
         *   '0020' . '10FFFF' - '"' - '\'
         *   '\' escape
         *
         * escape:
         *   '"'
         *   '\'
         *   '/'
         *   'b'
         *   'f'
         *   'n'
         *   'r'
         *   't'
         *   'u' hex hex hex hex
         *
         * hex:
         *   digit
         *   'A' . 'F'
         *   'a' . 'f'
         */
        Either<JsonString, JsonParseError> JsonParser::parseString(JsonParseContext& ctx) {
            spargel_trace_scope("parseString");

            auto& cursor = ctx.cursor;

            // '"'
            if (!cursorTryEatChar(cursor, '"')) return Right(JsonParseError("expected '\"'"_sv));

            // characters
            base::vector<char> chars;

            while (!cursorIsEnd(cursor)) {
                char ch = (char)cursorGetChar(cursor);

                if (isGood(ch)) {
                    chars.emplace(ch);
                    continue;
                }

                // '"'
                if (ch == '"') {
                    return Left(JsonString(base::string_from_range(chars.begin(), chars.end())));
                }

                // '\'
                if (ch == '\\') {
                    if (cursorIsEnd(cursor)) return Right(JsonParseError("unfinished escape"_sv));

                    ch = (char)cursorGetChar(cursor);
                    switch (ch) {
                    case '\"':
                    case '\\':
                    case '/':  // why escape this?
                        chars.emplace(ch);
                        break;
                    case 'b':
                        chars.emplace('\b');
                        break;
                    case 'f':
                        chars.emplace('\f');
                        break;
                    case 'n':
                        chars.emplace('\n');
                        break;
                    case 'r':
                        chars.emplace('\r');
                        break;
                    case 't':
                        chars.emplace('\t');
                        break;
                    case 'u': {
                        // TODO: unicode
                        u16 code = 0;
                        for (int i = 0; i < 4; i++) {
                            if (cursorIsEnd(cursor))
                                return Right(JsonParseError("expected a hex digit"_sv));
                            ch = (char)cursorGetChar(cursor);
                            char v;
                            if ('0' <= ch && ch <= '9')
                                v = ch - '0';
                            else if ('A' <= ch && ch <= 'F')
                                v = ch - 'A' + 0xa;
                            else if ('a' <= ch && ch <= 'f')
                                v = ch - 'a' + 0xa;
                            else
                                return Right(JsonParseError("bad hex digit"_sv));
                            code = (u16)(code * 0x10 + v);
                        }
                        appendUtf8(chars, code);
                    } break;
                    default:
                        return Right(
                            JsonParseError(String("unexpected escape character: '") + ch + '\''));
                    }
                } else if ((u8)ch >= 0x20) {
                    // TODO: unicode
                    // no problem for UTF-8
                    chars.emplace(ch);
                } else {
                    return Right(JsonParseError(String("invalid character 0x") + char2hex(ch)));
                }
            }

            return Right(JsonParseError(UNEXPECTED_END));
        }

        /*
         * integer:
         *   digit
         *   onenine digits
         *   '-' digit
         *   '-' onenine digits
         *
         * digits:
         *   digit
         *   digit digits
         *
         * digit:
         *   '0'
         *   onenine
         *
         * onenine:
         *   '1' . '9'
         */
        Optional<JsonParseError> JsonParser::parseInteger(JsonParseContext& ctx, JsonNumber& number,
                                                          bool& minus) {
            spargel_trace_scope("parseInteger");

            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return makeOptional<JsonParseError>(UNEXPECTED_END);

            minus = false;
            char ch = (char)cursorPeek(cursor);

            // '-'
            if (ch == '-') {
                minus = true;
                cursorAdvance(cursor);
                ch = (char)cursorPeek(cursor);
            }

            // digit
            if (ch == '0') {
                cursorAdvance(cursor);
                return nullopt;
            }

            // digit
            // onenine digits
            if ('0' < ch && ch <= '9') {
                number = (ch - '0');
                cursorAdvance(cursor);
                while (!cursorIsEnd(cursor)) {
                    ch = (char)cursorPeek(cursor);
                    if ('0' <= ch && ch <= '9') {
                        number = number * 10 + (ch - '0');
                        cursorAdvance(cursor);
                    } else {
                        break;
                    }
                }

                return nullopt;
            }

            return makeOptional<JsonParseError>("expected a number"_sv);
        }

        /*
         * fraction:
         *   ""
         *   '.' digits
         */
        Optional<JsonParseError> JsonParser::parseFraction(JsonParseContext& ctx,
                                                           JsonNumber& number) {
            spargel_trace_scope("parseFraction");

            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return nullopt;

            // '.'
            if (cursorPeek(cursor) != '.') return nullopt;
            cursorAdvance(cursor);

            // digits
            char ch = (char)cursorPeek(cursor);
            if (!('0' <= ch && ch <= '9'))
                return makeOptional<JsonParseError>("expected fractional part"_sv);
            JsonNumber x = 0.1;
            while (!cursorIsEnd(cursor)) {
                ch = (char)cursorPeek(cursor);
                if ('0' <= ch && ch <= '9') {
                    number += x * (ch - '0');
                    x *= 0.1;
                    cursorAdvance(cursor);
                } else {
                    break;
                }
            }

            return nullopt;
        }

        /*
         * exponent:
         *   ""
         *   'E' sign digits
         *   'e' sign digits
         *
         * sign:
         *   ""
         *   '+'
         *   '-'
         */
        Optional<JsonParseError> JsonParser::parseExponent(JsonParseContext& ctx,
                                                           JsonNumber& number) {
            spargel_trace_scope("parseExponent");

            auto& cursor = ctx.cursor;

            if (!cursorTryEatChar(cursor, 'E') && !cursorTryEatChar(cursor, 'e')) return nullopt;

            bool minus = false;
            if (cursorTryEatChar(cursor, '-')) {
                minus = true;
            } else if (cursorTryEatChar(cursor, '+')) {
                // skip
            }

            // digits
            char ch = (char)cursorPeek(cursor);
            if (!('0' <= ch && ch <= '9'))
                return makeOptional<JsonParseError>("expected exponential part"_sv);
            JsonNumber exponent = 0;
            while (!cursorIsEnd(cursor)) {
                ch = (char)cursorPeek(cursor);
                if ('0' <= ch && ch <= '9') {
                    exponent = exponent * 10 + (ch - '0');
                    cursorAdvance(cursor);
                } else {
                    break;
                }
            }

            if (minus)
                number *= pow(0.1, exponent);
            else
                number *= pow(10, exponent);

            return nullopt;
        }

        /*
         * number:
         *   integer fraction exponent
         */
        Either<JsonNumber, JsonParseError> JsonParser::parseNumber(JsonParseContext& ctx) {
            spargel_trace_scope("parseNumber");

            JsonNumber number(0);
            bool minus;
            auto result = parseInteger(ctx, number, minus);
            if (result.hasValue()) return Right(base::move(result.value()));

            result = parseFraction(ctx, number);
            if (result.hasValue()) return Right(base::move(result.value()));

            result = parseExponent(ctx, number);
            if (result.hasValue()) return Right(base::move(result.value()));

            if (minus) number = -number;

            return Left(base::move(number));
        }

        /*
         *   "true"
         *   "false"
         */
        Either<JsonBoolean, JsonParseError> JsonParser::parseBoolean(JsonParseContext& ctx) {
            spargel_trace_scope("parseBoolean");

            auto& cursor = ctx.cursor;

            if (cursorTryEatString(cursor, "true")) {
                return Left(JsonBoolean(true));
            } else if (cursorTryEatString(cursor, "false")) {
                return Left(JsonBoolean(false));
            } else {
                return Right(JsonParseError("expected \"true\" or \"false\""_sv));
            }
        }

        /*
         *   "null"
         */
        Optional<JsonParseError> JsonParser::parseNull(JsonParseContext& ctx) {
            spargel_trace_scope("parseNull");

            auto& cursor = ctx.cursor;

            if (cursorTryEatString(cursor, "null"))
                return nullopt;
            else
                return makeOptional<JsonParseError>("expected \"null\""_sv);
        }

        /*
         * members:
         *   member
         *   member ',' members
         */
        Either<JsonObject, JsonParseError> JsonParser::parseMembers(JsonParseContext& ctx) {
            spargel_trace_scope("parseMembers");

            auto& cursor = ctx.cursor;

            base::HashMap<JsonString, JsonValue> members;
            while (!cursorIsEnd(cursor)) {
                // member
                JsonString key;
                JsonValue value;
                auto result = parseMember(ctx, key, value);
                if (result.hasValue()) return Right(base::move(result.value()));

                members.set(base::move(key), base::move(value));

                // ','
                if (!cursorTryEatChar(cursor, ',')) return Left(JsonObject(base::move(members)));
            }

            return Right(JsonParseError(UNEXPECTED_END));
        }

        /*
         * member:
         *   ws string ws ':' element
         */
        Optional<JsonParseError> JsonParser::parseMember(JsonParseContext& ctx, JsonString& key,
                                                         JsonValue& value) {
            spargel_trace_scope("parseMember");

            auto& cursor = ctx.cursor;

            // ws
            eatWhitespaces(ctx);

            // string
            auto result_key = parseString(ctx);
            if (result_key.isRight())
                return makeOptional<JsonParseError>(base::move(result_key.right()));
            key = base::move(result_key.left());

            // ws
            eatWhitespaces(ctx);

            // ':'
            if (!cursorTryEatChar(cursor, ':'))
                return makeOptional<JsonParseError>("expected ':'"_sv);

            // element
            auto result_value = parseElement(ctx);
            if (result_value.isRight())
                return makeOptional<JsonParseError>(base::move(result_value.right()));
            value = base::move(result_value.left());

            return nullopt;
        }

        /*
         * elements:
         *   element
         *   element ',' elements
         */
        Either<JsonArray, JsonParseError> JsonParser::parseElements(JsonParseContext& ctx) {
            spargel_trace_scope("parseElements");

            auto& cursor = ctx.cursor;

            base::vector<JsonValue> elements;
            while (!cursorIsEnd(cursor)) {
                // element
                auto result = parseElement(ctx);
                if (result.isRight()) return Right(base::move(result.right()));

                elements.emplace(base::move(result.left()));

                // ','
                if (!cursorTryEatChar(cursor, ',')) return Left(JsonArray(base::move(elements)));
            }

            return Right(JsonParseError(UNEXPECTED_END));
        }

        /*
         * element:
         *   ws value ws
         */
        Either<JsonValue, JsonParseError> JsonParser::parseElement(JsonParseContext& ctx) {
            spargel_trace_scope("parseElement");

            // ws
            eatWhitespaces(ctx);

            // value
            auto result = parseValue(ctx);
            if (result.isRight()) return result;

            // ws
            eatWhitespaces(ctx);

            return result;
        }

    }  // namespace

    Either<JsonValue, JsonParseError> parseJson(const char* str, usize length) {
        spargel_trace_scope("parseJson");

        JsonParser::JsonParseContext ctx;
        ctx.cursor.cur = str;
        ctx.cursor.end = str + length;
        return JsonParser{}.parseElement(ctx);
    }

}  // namespace spargel::json
