#include <spargel/codec/cursor.h>
#include <spargel/codec/json.h>

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

        struct JsonParseContext {
            Cursor cursor;
        };

        JsonParseResult parseValue(JsonParseContext& ctx, JsonValue& value);
        JsonParseResult parseObject(JsonParseContext& ctx, JsonObject& object);
        JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array);
        JsonParseResult parseString(JsonParseContext& ctx, JsonString& string);
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
        void eatWhitespace(JsonParseContext& ctx) {
            auto& cursor = ctx.cursor;
            while (!cursorIsEnd(cursor)) {
                char ch = cursorPeek(cursor);
                if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
                    cursorAdvance(cursor);
                else
                    break;
                ;
            }
        }

        /*
         * value:
         *   object
         *   array
         *   string
         *   number (TODO)
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
                result =
                    JsonParseResult::error(base::string("unexpected character: '") + ch + '\'');
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
            eatWhitespace(ctx);
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
         * array
         *   '[' ws ']'
         *   '[' elements ']'
         */
        JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array) {
            auto& cursor = ctx.cursor;

            // '[' ws
            if (!cursorTryEatChar(cursor, '[')) return JsonParseResult::error("expected '['");
            eatWhitespace(ctx);
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
         * string
         *   '"' characters '"'
         */
        JsonParseResult parseString(JsonParseContext& ctx, JsonString& string) {
            auto& cursor = ctx.cursor;

            // '"'
            if (!cursorTryEatChar(cursor, '"')) return JsonParseResult::error("expected '\"'");

            // characters
            const char* begin = cursor.cur;
            while (!cursorIsEnd(cursor)) {
                if (cursorGetChar(cursor) == '"') {
                    string = base::string_from_range(begin, cursor.cur - 1);
                    return SUCCESS;
                }
            }

            return UNEXPECTED_END;
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
            eatWhitespace(ctx);

            // string
            auto result = parseString(ctx, key);
            if (result.failed()) return result;

            // ws
            eatWhitespace(ctx);

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
            eatWhitespace(ctx);

            // value
            auto result = parseValue(ctx, value);
            if (result.failed()) return result;

            // ws
            eatWhitespace(ctx);

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
