#include <spargel/codec/json.h>

namespace spargel::codec {

    const static JsonParseResult SUCCESS = JsonParseResult::success();

    JsonObject* getJsonMemberObject(JsonObject& json, const JsonString& key,
                                    JsonParseResult& result, bool optional) {
        auto* member = json.members.get(key);
        if (member == nullptr) {
            result = optional
                         ? SUCCESS
                         : JsonParseResult::error(base::string("expected member \"") + key + '"');

            return nullptr;
        } else {
            if (member->type == JsonValueType::object) {
                result = SUCCESS;
                return &member->object;
            } else {
                result = JsonParseResult::error(base::string("expected \"") + key +
                                                "\" to be an object");
                return nullptr;
            }
        }
    }

    JsonArray* getJsonMemberArray(JsonObject& json, const JsonString& key, JsonParseResult& result,
                                  bool optional) {
        auto* member = json.members.get(key);
        if (member == nullptr) {
            result = optional
                         ? SUCCESS
                         : JsonParseResult::error(base::string("expected member \"") + key + '"');

            return nullptr;
        } else {
            if (member->type == JsonValueType::array) {
                result = SUCCESS;
                return &member->array;
            } else {
                result =
                    JsonParseResult::error(base::string("expected \"") + key + "\" to be an array");
                return nullptr;
            }
        }
    }

    JsonString* getJsonMemberString(JsonObject& json, const JsonString& key,
                                    JsonParseResult& result, bool optional) {
        auto* member = json.members.get(key);
        if (member == nullptr) {
            result = optional
                         ? SUCCESS
                         : JsonParseResult::error(base::string("expected member \"") + key + '"');

            return nullptr;
        } else {
            if (member->type == JsonValueType::string) {
                result = SUCCESS;
                return &member->string;
            } else {
                result =
                    JsonParseResult::error(base::string("expected \"") + key + "\" to be a string");
                return nullptr;
            }
        }
    }

    JsonNumber* getJsonMemberNumber(JsonObject& json, const JsonString& key,
                                    JsonParseResult& result, bool optional) {
        auto* member = json.members.get(key);
        if (member == nullptr) {
            result = optional
                         ? SUCCESS
                         : JsonParseResult::error(base::string("expected member \"") + key + '"');

            return nullptr;
        } else {
            if (member->type == JsonValueType::number) {
                result = SUCCESS;
                return &member->number;
            } else {
                result =
                    JsonParseResult::error(base::string("expected \"") + key + "\" to be a number");
                return nullptr;
            }
        }
    }

}  // namespace spargel::codec
