#include <spargel/codec/cursor.h>
#include <spargel/codec/json.h>

namespace spargel::codec {

    JsonValue::JsonValue(const JsonValue& other) {
        this->~JsonValue();
        type = other.type;
        switch (other.type) {
        case JsonValueType::object:
            object = other.object;
            break;
        case JsonValueType::array:
            array = other.array;
            break;
        case JsonValueType::string:
            string = other.string;
            break;
        case JsonValueType::number:
            number = other.number;
            break;
        case JsonValueType::boolean:
            boolean = other.boolean;
        case JsonValueType::null:
            break;
        }
    }

    JsonValue::~JsonValue() {
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
        // These types do not need to be destructed
        case JsonValueType::number:
        case JsonValueType::boolean:
        case JsonValueType::null:
            break;
        }
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
                value.type = JsonValueType::object;
                new (&value.object) JsonObject;
                result = parseObject(ctx, value.object);
                break;
            case '[':
                /* array */
                value.type = JsonValueType::array;
                new (&value.array) JsonArray;
                result = parseArray(ctx, value.array);
                break;
            case '"':
                /* string */
                value.type = JsonValueType::string;
                new (&value.string) JsonString;
                result = parseString(ctx, value.string);
                break;
            case 't':
            case 'f':
                /* true/false */
                value.type = JsonValueType::boolean;
                result = parseBoolean(ctx, value.boolean);
                break;
            case 'n':
                /* null */
                value.type = JsonValueType::null;
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

            return JsonParseResult::success();
        }

    }  // namespace

    JsonParseResult parseJson(const char* str, usize length, JsonValue& value) {
        JsonParseContext ctx;
        ctx.cursor.cur = str;
        ctx.cursor.end = str + length;
        return parseElement(ctx, value);
    }

}  // namespace spargel::codec
