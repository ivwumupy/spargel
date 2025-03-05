#include <spargel/codec/json.h>

namespace spargel::codec {

    namespace {

        base::Either<const JsonValue*, JsonParseError> getJsonMember(const JsonObject& json,
                                                                     const JsonString& key,
                                                                     JsonValueType type,
                                                                     const base::string& type_name,
                                                                     bool optional) {
            auto* member = json.members.get(key);
            if (member == nullptr) {
                if (optional)
                    return base::makeLeft<const JsonValue*, JsonParseError>(nullptr);
                else
                    return base::makeRight<const JsonValue*, JsonParseError>(
                        base::string("expected member \"") + key + '"');
            } else {
                if (member->type == type) {
                    return base::makeLeft<const JsonValue*, JsonParseError>(member);
                } else {
                    return base::makeRight<const JsonValue*, JsonParseError>(
                        base::string("expected \"") + key + "\" to be of type \"" + type_name +
                        '"');
                }
            }
        }

    }  // namespace

    base::Either<const JsonObject*, JsonParseError> getJsonMemberObject(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional) {
        auto result =
            getJsonMember(json, key, JsonValueType::object, base::string("object"), optional);
        if (result.isRight())
            return base::makeRight<const JsonObject*, JsonParseError>(result.right());
        else
            return base::makeLeft<const JsonObject*, JsonParseError>(&result.left()->object);
    }

    base::Either<const JsonArray*, JsonParseError> getJsonMemberArray(const JsonObject& json,
                                                                      const JsonString& key,
                                                                      bool optional) {
        auto result =
            getJsonMember(json, key, JsonValueType::array, base::string("array"), optional);
        if (result.isRight())
            return base::makeRight<const JsonArray*, JsonParseError>(result.right());
        else
            return base::makeLeft<const JsonArray*, JsonParseError>(&result.left()->array);
    }

    base::Either<const JsonString*, JsonParseError> getJsonMemberString(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional) {
        auto result =
            getJsonMember(json, key, JsonValueType::string, base::string("string"), optional);
        if (result.isRight())
            return base::makeRight<const JsonString*, JsonParseError>(result.right());
        else
            return base::makeLeft<const JsonString*, JsonParseError>(&result.left()->string);
    }

    base::Either<const JsonNumber*, JsonParseError> getJsonMemberNumber(const JsonObject& json,
                                                                        const JsonString& key,
                                                                        bool optional) {
        auto result =
            getJsonMember(json, key, JsonValueType::number, base::string("number"), optional);
        if (result.isRight())
            return base::makeRight<const JsonNumber*, JsonParseError>(result.right());
        else
            return base::makeLeft<const JsonNumber*, JsonParseError>(&result.left()->number);
    }

}  // namespace spargel::codec
