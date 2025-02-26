#include <spargel/codec/cursor.h>
#include <spargel/codec/json.h>

// libm
#include <math.h>

namespace spargel::codec {

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

        static const JsonParseResult SUCCESS = JsonParseResult::success();
        static const JsonParseResult UNEXPECTED_END = JsonParseResult::error("unexpected end");

        base::string char2hex(char ch) {
            const char hexDigits[] = "0123456789abcdef";
            return base::string(hexDigits[(ch >> 4) & 0xf]) + hexDigits[ch & 0xf];
        }

        struct JsonParseContext {
            Cursor cursor;
        };

        JsonParseResult parseValue(JsonParseContext& ctx, JsonValue& value);
        JsonParseResult parseObject(JsonParseContext& ctx, JsonObject& object);
        JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array);
        JsonParseResult parseString(JsonParseContext& ctx, JsonString& string);
        JsonParseResult parseNumber(JsonParseContext& ctx, JsonNumber& number);
        JsonParseResult parseBoolean(JsonParseContext& ctx, JsonBoolean& boolean);
        JsonParseResult parseNull(JsonParseContext& ctx);
        JsonParseResult parseMembers(JsonParseContext& ctx, JsonObject& object);
        JsonParseResult parseMember(JsonParseContext& ctx, JsonString& key, JsonValue& value);
        JsonParseResult parseElements(JsonParseContext& ctx, JsonArray& array);
        JsonParseResult parseElement(JsonParseContext& ctx, JsonValue& value);

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
        void eatWhitespaces(JsonParseContext& ctx) {
            auto& cursor = ctx.cursor;
            while (!cursorIsEnd(cursor)) {
                char ch = cursorPeek(cursor);
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
        JsonParseResult parseValue(JsonParseContext& ctx, JsonValue& value) {
            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return JsonParseResult::error("expected a value");

            char ch = cursorPeek(cursor);
            JsonParseResult result;
            switch (ch) {
            case '{':
                /* object */
                value = JsonValue(JsonObject());
                result = parseObject(ctx, value.object);
                break;
            case '[':
                /* array */
                value = JsonValue(JsonArray());
                result = parseArray(ctx, value.array);
                break;
            case '"':
                /* string */
                value = JsonValue(JsonString());
                result = parseString(ctx, value.string);
                break;
            case 't':
            case 'f':
                /* true/false */
                value = JsonValue(JsonBoolean());
                result = parseBoolean(ctx, value.boolean);
                break;
            case 'n':
                /* null */
                value = JsonValue();
                result = parseNull(ctx);
                break;
            default:
                /* number */
                if ((ch >= '0' && ch <= '9') || ch == '-') {
                    value = JsonValue(JsonNumber());
                    result = parseNumber(ctx, value.number);
                } else {
                    result =
                        JsonParseResult::error(base::string("unexpected character: '") + ch + '\'');
                }
            }

            return result;
        }

        /*
         * object:
         *   '{' ws '}'
         *   '{' members '}'
         */
        JsonParseResult parseObject(JsonParseContext& ctx, JsonObject& object) {
            auto& cursor = ctx.cursor;

            // '{' ws
            if (!cursorTryEatChar(cursor, '{')) return JsonParseResult::error("expected '{'");
            eatWhitespaces(ctx);
            if (cursorIsEnd(cursor)) return UNEXPECTED_END;

            // '}'
            if (cursorTryEatChar(cursor, '}')) return SUCCESS;

            // members '}'
            auto result = parseMembers(ctx, object);
            if (result.failed()) return result;
            if (!cursorTryEatChar(cursor, '}')) return JsonParseResult::error("expected '}'");

            return SUCCESS;
        }

        /*
         * array:
         *   '[' ws ']'
         *   '[' elements ']'
         */
        JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array) {
            auto& cursor = ctx.cursor;

            // '[' ws
            if (!cursorTryEatChar(cursor, '[')) return JsonParseResult::error("expected '['");
            eatWhitespaces(ctx);
            if (cursorIsEnd(cursor)) return UNEXPECTED_END;

            // ']'
            if (cursorTryEatChar(cursor, ']')) return SUCCESS;

            // elements ']'
            auto result = parseElements(ctx, array);
            if (result.failed()) return result;
            if (!cursorTryEatChar(cursor, ']')) return JsonParseResult::error("expected ']'");

            return SUCCESS;
        }

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
        JsonParseResult parseString(JsonParseContext& ctx, JsonString& string) {
            auto& cursor = ctx.cursor;

            // '"'
            if (!cursorTryEatChar(cursor, '"')) return JsonParseResult::error("expected '\"'");

            // characters
            base::vector<char> chars;
            while (!cursorIsEnd(cursor)) {
                char ch = cursorGetChar(cursor);
                // '"'
                if (ch == '"') {
                    string = base::string_from_range(chars.begin(), chars.end());
                    return SUCCESS;
                }

                // '\'
                if (ch == '\\') {
                    if (cursorIsEnd(cursor)) return JsonParseResult::error("unfinished escape");

                    ch = cursorGetChar(cursor);
                    switch (ch) {
                    case '\"':
                    case '\\':
                    case '/':  // why escape this?
                        chars.push(ch);
                        break;
                    case 'b':
                        chars.push('\b');
                        break;
                    case 'f':
                        chars.push('\f');
                        break;
                    case 'n':
                        chars.push('\n');
                        break;
                    case 'r':
                        chars.push('\r');
                        break;
                    case 't':
                        chars.push('\t');
                        break;
                    case 'u': {
                        // TODO: unicode
                        u16 code = 0;
                        for (int i = 0; i < 4; i++) {
                            if (cursorIsEnd(cursor))
                                return JsonParseResult::error("expected a hex digit");
                            ch = cursorGetChar(cursor);
                            u8 v;
                            if ('0' <= ch && ch <= '9')
                                v = ch - '0';
                            else if ('A' <= ch && ch <= 'F')
                                v = ch - 'A' + 0x10;
                            else if ('a' <= ch && ch <= 'f')
                                v = ch - 'a' + 0x10;
                            else
                                return JsonParseResult::error("bad hex digit");
                            code = code * 0x10 + v;
                        }
                        chars.push(code & 0xff);
                    } break;
                    default:
                        return JsonParseResult::error(
                            base::string("unexpected escape character: '") + ch + '\'');
                    }
                } else if ((u8)ch >= 0x20) {
                    // TODO: unicode
                    // no problem for UTF-8
                    chars.push(ch);
                } else {
                    return JsonParseResult::error(base::string("invalid character 0x") +
                                                  char2hex(ch));
                }
            }

            return UNEXPECTED_END;
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
        JsonParseResult parseInteger(JsonParseContext& ctx, JsonNumber& number, bool& minus) {
            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return UNEXPECTED_END;

            minus = false;
            char ch = cursorPeek(cursor);

            // '-'
            if (ch == '-') {
                minus = true;
                cursorAdvance(cursor);
                ch = cursorPeek(cursor);
            }

            // digit
            if (ch == '0') {
                cursorAdvance(cursor);
                return SUCCESS;
            }

            // digit
            // onenine digits
            if (ch > '0' && ch <= '9') {
                number = (ch - '0');
                cursorAdvance(cursor);
                while (!cursorIsEnd(cursor)) {
                    ch = cursorPeek(cursor);
                    if (ch >= '0' && ch <= '9') {
                        number = number * 10 + (ch - '0');
                        cursorAdvance(cursor);
                    } else {
                        break;
                    }
                }

                return SUCCESS;
            }

            return JsonParseResult::error("expected a number");
        }

        /*
         * fraction:
         *   ""
         *   '.' digits
         */
        JsonParseResult parseFraction(JsonParseContext& ctx, JsonNumber& number) {
            auto& cursor = ctx.cursor;
            if (cursorIsEnd(cursor)) return SUCCESS;

            // '.'
            if (cursorPeek(cursor) != '.') return SUCCESS;
            cursorAdvance(cursor);

            // digits
            char ch = cursorPeek(cursor);
            if (!(ch >= '0' && ch <= '9'))
                return JsonParseResult::error("expected fractional part");
            JsonNumber x = 0.1;
            while (!cursorIsEnd(cursor)) {
                ch = cursorPeek(cursor);
                if (ch >= '0' && ch <= '9') {
                    number += x * (ch - '0');
                    x *= 0.1;
                    cursorAdvance(cursor);
                } else {
                    break;
                }
            }

            return SUCCESS;
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
        JsonParseResult parseExponent(JsonParseContext& ctx, JsonNumber& number) {
            auto& cursor = ctx.cursor;

            if (!cursorTryEatChar(cursor, 'E') && !cursorTryEatChar(cursor, 'e')) return SUCCESS;

            bool minus = false;
            if (cursorTryEatChar(cursor, '-')) {
                minus = true;
            } else if (cursorTryEatChar(cursor, '+')) {
                // skip
            }

            // digits
            char ch = cursorPeek(cursor);
            if (!(ch >= '0' && ch <= '9'))
                return JsonParseResult::error("expected exponential part");
            JsonNumber exponent = 0;
            while (!cursorIsEnd(cursor)) {
                ch = cursorPeek(cursor);
                if (ch >= '0' && ch <= '9') {
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

            return SUCCESS;
        }

        /*
         * number:
         *   integer fraction exponent
         */
        JsonParseResult parseNumber(JsonParseContext& ctx, JsonNumber& number) {
            bool minus;
            auto result = parseInteger(ctx, number, minus);
            if (result.failed()) return result;

            result = parseFraction(ctx, number);
            if (result.failed()) return result;

            result = parseExponent(ctx, number);
            if (result.failed()) return result;

            if (minus) number = -number;

            return SUCCESS;
        }

        /*
         *   "true"
         *   "false"
         */
        JsonParseResult parseBoolean(JsonParseContext& ctx, JsonBoolean& boolean) {
            auto& cursor = ctx.cursor;

            if (cursorTryEatString(cursor, "true")) {
                boolean = true;
                return SUCCESS;
            } else if (cursorTryEatString(cursor, "false")) {
                boolean = false;
                return SUCCESS;
            } else {
                return JsonParseResult::error("expected \"true\" or \"false\"");
            }
        }

        /*
         *   "null"
         */
        JsonParseResult parseNull(JsonParseContext& ctx) {
            auto& cursor = ctx.cursor;

            if (cursorTryEatString(cursor, "null"))
                return SUCCESS;
            else
                return JsonParseResult::error("expected \"null\"");
        }

        /*
         * members:
         *   member
         *   member ',' members
         */
        JsonParseResult parseMembers(JsonParseContext& ctx, JsonObject& object) {
            auto& cursor = ctx.cursor;

            JsonParseResult result;
            while (!cursorIsEnd(cursor)) {
                // member
                JsonString key;
                JsonValue value;
                result = parseMember(ctx, key, value);
                if (result.failed()) return result;

                object.members.set(base::move(key), base::move(value));

                // ','
                if (!cursorTryEatChar(cursor, ',')) return SUCCESS;
            }

            return UNEXPECTED_END;
        }

        /*
         * member:
         *   ws string ws ':' element
         */
        JsonParseResult parseMember(JsonParseContext& ctx, JsonString& key, JsonValue& value) {
            auto& cursor = ctx.cursor;

            // ws
            eatWhitespaces(ctx);

            // string
            auto result = parseString(ctx, key);
            if (result.failed()) return result;

            // ws
            eatWhitespaces(ctx);

            // ':'
            if (!cursorTryEatChar(cursor, ':')) return JsonParseResult::error("expected ':'");

            // element
            result = parseElement(ctx, value);
            if (result.failed()) return result;

            return SUCCESS;
        }

        /*
         * elements:
         *   element
         *   element ',' elements
         */
        JsonParseResult parseElements(JsonParseContext& ctx, JsonArray& array) {
            auto& cursor = ctx.cursor;

            JsonParseResult result;
            while (!cursorIsEnd(cursor)) {
                // element
                JsonValue value;
                result = parseElement(ctx, value);
                if (result.failed()) return result;

                array.elements.push(base::move(value));

                // ','
                if (!cursorTryEatChar(cursor, ',')) return SUCCESS;
            }

            return UNEXPECTED_END;
        }

        /*
         * element:
         *   ws value ws
         */
        JsonParseResult parseElement(JsonParseContext& ctx, JsonValue& value) {
            // ws
            eatWhitespaces(ctx);

            // value
            auto result = parseValue(ctx, value);
            if (result.failed()) return result;

            // ws
            eatWhitespaces(ctx);

            return SUCCESS;
        }

    }  // namespace

    JsonParseResult parseJson(const char* str, usize length, JsonValue& value) {
        JsonParseContext ctx;
        ctx.cursor.cur = str;
        ctx.cursor.end = str + length;
        return parseElement(ctx, value);
    }

}  // namespace spargel::codec
