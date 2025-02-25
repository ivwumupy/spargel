
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

    static const JsonParseResult SUCCESS = JsonParseResult::success();
    static const JsonParseResult UNEXPECTED_END = JsonParseResult::error("unexpected end");

    struct JsonParseContext {
        Cursor cursor;
    };

    static JsonParseResult parseValue(JsonParseContext& ctx, JsonValue& value);
    static JsonParseResult parseObject(JsonParseContext& ctx, JsonObject& object);
    static JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array);
    static JsonParseResult parseString(JsonParseContext& ctx, JsonString& string);
    static JsonParseResult parseBoolean(JsonParseContext& ctx, JsonBoolean& boolean);
    static JsonParseResult parseNull(JsonParseContext& ctx);
    static JsonParseResult parseMembers(JsonParseContext& ctx, JsonObject& object);
    static JsonParseResult parseMember(JsonParseContext& ctx, JsonString& key, JsonValue& value);
    static JsonParseResult parseElements(JsonParseContext& ctx, JsonArray& array);
    static JsonParseResult parseElement(JsonParseContext& ctx, JsonValue& value);

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
    static void eatWhitespace(JsonParseContext& ctx) {
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
    static JsonParseResult parseValue(JsonParseContext& ctx, JsonValue& value) {
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
            result = JsonParseResult::error(base::string("unexpected character: '") + ch + '\'');
        }

        return result;
    }

    /*
     * object:
     *   '{' ws '}'
     *   '{' members '}'
     */
    static JsonParseResult parseObject(JsonParseContext& ctx, JsonObject& object) {
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
    static JsonParseResult parseArray(JsonParseContext& ctx, JsonArray& array) {
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
    static JsonParseResult parseString(JsonParseContext& ctx, JsonString& string) {
        auto& cursor = ctx.cursor;

        // '"'
        if (!cursorTryEatChar(cursor, '"')) return JsonParseResult::error("expected '\"'");

        // characters
        const char* begin = cursor.cur;
        const char* end = cursor.cur;
        while (!cursorIsEnd(cursor)) {
            if (cursorGetChar(cursor) == '"') {
                end = cursor.cur;
                string = base::string_from_range(begin, end);
                return SUCCESS;
            }
        }

        return UNEXPECTED_END;
    }

    /*
     *   "true"
     *   "false"
     */
    static JsonParseResult parseBoolean(JsonParseContext& ctx, JsonBoolean& boolean) {
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
    static JsonParseResult parseNull(JsonParseContext& ctx) {
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
    static JsonParseResult parseMembers(JsonParseContext& ctx, JsonObject& object) {
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
    static JsonParseResult parseMember(JsonParseContext& ctx, JsonString& key, JsonValue& value) {
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
    static JsonParseResult parseElements(JsonParseContext& ctx, JsonArray& array) {
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
    static JsonParseResult parseElement(JsonParseContext& ctx, JsonValue& value) {
        // ws
        eatWhitespace(ctx);

        // value
        auto result = parseValue(ctx, value);
        if (result.failed()) return result;

        // ws
        eatWhitespace(ctx);

        return JsonParseResult::success();
    }

    JsonParseResult parseJson(const char* str, usize length, JsonValue& value) {
        JsonParseContext ctx;
        ctx.cursor.cur = str;
        ctx.cursor.end = str + length;
        return parseElement(ctx, value);
    }

}  // namespace spargel::codec
