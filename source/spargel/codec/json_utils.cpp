#include <spargel/codec/json.h>

namespace spargel::codec {

    const static JsonParseResult SUCCESS = JsonParseResult::success();

    namespace {

        const JsonValue* getJsonMember(const JsonObject& json, const JsonString& key,
                                       JsonValueType type, const base::string& type_name,
                                       JsonParseResult& result, bool optional) {
            auto* member = json.members.get(key);
            if (member == nullptr) {
                if (optional)
                    result = SUCCESS;
                else
                    result = JsonParseResult::error(base::string("expected member \"") + key + '"');

                return nullptr;
            } else {
                if (member->type == type) {
                    result = SUCCESS;
                    return member;
                } else {
                    result = JsonParseResult::error(base::string("expected \"") + key +
                                                    "\" to be of type \"" + type_name + '"');
                    return nullptr;
                }
            }
        }

    }  // namespace

    const JsonObject* getJsonMemberObject(const JsonObject& json, const JsonString& key,
                                          JsonParseResult& result, bool optional) {
        auto* member = getJsonMember(json, key, JsonValueType::object, base::string("object"),
                                     result, optional);
        return member == nullptr ? nullptr : &member->object;
    }

    const JsonArray* getJsonMemberArray(const JsonObject& json, const JsonString& key,
                                        JsonParseResult& result, bool optional) {
        auto* member =
            getJsonMember(json, key, JsonValueType::array, base::string("array"), result, optional);
        return member == nullptr ? nullptr : &member->array;
    }

    const JsonString* getJsonMemberString(const JsonObject& json, const JsonString& key,
                                          JsonParseResult& result, bool optional) {
        auto* member = getJsonMember(json, key, JsonValueType::string, base::string("string"),
                                     result, optional);
        return member == nullptr ? nullptr : &member->string;
    }

    const JsonNumber* getJsonMemberNumber(const JsonObject& json, const JsonString& key,
                                          JsonParseResult& result, bool optional) {
        auto* member = getJsonMember(json, key, JsonValueType::number, base::string("number"),
                                     result, optional);
        return member == nullptr ? nullptr : &member->number;
    }

}  // namespace spargel::codec
