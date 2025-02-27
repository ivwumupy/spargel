#include <spargel/codec/model/gltf.h>

namespace spargel::codec::model {

    using base::makeOptional;

    GlTFParseResult operator+(const GlTFParseResult& result1, const GlTFParseResult& result2) {
        if (!result1.failed() && !result2.failed())
            return GlTFParseResult::success();
        else
            return GlTFParseResult::error(result1.message() + " : " + result2.message());
    }

    namespace {

        const GlTFParseResult SUCCESS = GlTFParseResult::success();
        const GlTFParseResult EXPECTED_OBJECT = GlTFParseResult::error("expected an object");

        /*
         * helper functions
         */

        GlTFParseResult parseInteger(const JsonNumber& number, GlTFInteger& n) {
            n = GlTFInteger(number);
            if (number == JsonNumber(n))
                return SUCCESS;
            else
                return GlTFParseResult::error("expected an integer");
        }

        GlTFParseResult parseInteger(const JsonValue& value, GlTFInteger& n) {
            if (value.type != JsonValueType::number)
                return GlTFParseResult::error("expected a number");

            return parseInteger(value.number, n);
        }

        GlTFParseResult parseNumberArray(const JsonArray& array, usize count,
                                         vector<GlTFNumber>& components) {
            if (array.elements.count() != count)
                return GlTFParseResult::error("incorrect number of components");

            components.resize(count);

            for (usize i = 0; i < count; i++) {
                auto& element = array.elements[i];
                if (element.type != JsonValueType::number)
                    return GlTFParseResult::error("expected a number");

                components[i] = GlTFNumber(element.number);
            }

            return SUCCESS;
        }

        GlTFParseResult parseVector3(const JsonArray& array, Vector3f& vec) {
            vector<double> components;
            auto result = parseNumberArray(array, 3, components);
            if (result.failed()) return result;

            vec.x = components[0];
            vec.y = components[1];
            vec.z = components[2];

            return SUCCESS;
        }

        GlTFParseResult parseVector4(const JsonArray& array, Vector4f& vec) {
            vector<double> components;
            auto result = parseNumberArray(array, 4, components);
            if (result.failed()) return result;

            vec.x = components[0];
            vec.y = components[1];
            vec.z = components[2];
            vec.w = components[3];

            return SUCCESS;
        }

        GlTFParseResult parseMatrix4(const JsonArray& array, Matrix4x4f& matrix) {
            vector<double> components;
            auto result = parseNumberArray(array, 16, components);
            if (result.failed()) return result;

            for (int i = 0; i < 16; i++) matrix.entries[i] = components[i];

            return SUCCESS;
        }

        GlTFParseResult parseMemberString(const JsonObject& object, const char* key,
                                          GlTFString& string) {
            JsonParseResult json_result;
            auto* ptr = getJsonMemberString(object, JsonString(key), json_result);
            if (json_result.failed()) return GlTFParseResult(json_result);

            string = GlTFString(*ptr);

            return SUCCESS;
        }

        GlTFParseResult parseMemberStringOptional(const JsonObject& object, const char* key,
                                                  Optional<GlTFString>& string) {
            JsonParseResult json_result;
            auto* ptr = getJsonMemberString(object, JsonString(key), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);

            if (ptr != nullptr)
                string = makeOptional<GlTFString>(*ptr);
            else
                string = Optional<GlTFString>();

            return SUCCESS;
        }

        GlTFParseResult parseMemberInteger(const JsonObject& object, const char* key,
                                           GlTFInteger& integer) {
            JsonParseResult json_result;
            auto* ptr = getJsonMemberNumber(object, JsonString(key), json_result);
            if (json_result.failed()) return GlTFParseResult(json_result);

            auto result = parseInteger(*ptr, integer);
            if (result.failed()) return result;

            return SUCCESS;
        }

        GlTFParseResult parseMemberIntegerOptional(const JsonObject& object, const char* key,
                                                   Optional<GlTFInteger>& integer) {
            JsonParseResult json_result;
            auto* ptr = getJsonMemberNumber(object, JsonString(key), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);

            if (ptr != nullptr) {
                GlTFInteger n;
                auto result = parseInteger(*ptr, n);
                if (result.failed()) return result;

                integer = makeOptional<GlTFInteger>(base::move(n));
            } else {
                integer = Optional<GlTFInteger>();
            }

            return SUCCESS;
        }

        GlTFParseResult parseMemberIntegerArrayOptional(const JsonObject& object, const char* key,
                                                        Optional<vector<GlTFInteger>>& array) {
            JsonParseResult json_result;
            auto* ptr = getJsonMemberArray(object, JsonString(key), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);

            if (ptr != nullptr) {
                vector<GlTFInteger> _array;
                for (auto& element : ptr->elements) {
                    GlTFInteger n;
                    auto result = parseInteger(element, n);
                    if (result.failed()) return result;

                    _array.push(base::move(n));
                }

                array = makeOptional<vector<GlTFInteger>>(base::move(_array));
            } else {
                array = Optional<vector<GlTFInteger>>();
            }

            return SUCCESS;
        }

        /*
         * parse sub-structures
         */

        GlTFParseResult parseAsset(const JsonObject& object, GlTFAsset& asset) {
            const auto ERROR = GlTFParseResult::error("parseAsset");
            GlTFParseResult result;

            // [copyright]
            result = parseMemberStringOptional(object, "copyright", asset.copyright);
            if (result.failed()) return ERROR + result;

            // [generator]
            result = parseMemberStringOptional(object, "generator", asset.generator);
            if (result.failed()) return ERROR + result;

            // version
            result = parseMemberString(object, "version", asset.version);
            if (result.failed()) return ERROR + result;

            // [minVersion]
            result = parseMemberStringOptional(object, "minVersion", asset.minVersion);
            if (result.failed()) return ERROR + result;

            return SUCCESS;
        }

        GlTFParseResult parseBuffer(const JsonObject& object, GlTFBuffer& buffer) {
            const auto ERROR = GlTFParseResult::error("parseBuffer");
            GlTFParseResult result;

            // [uri]
            result = parseMemberStringOptional(object, "uri", buffer.uri);
            if (result.failed()) return ERROR + result;

            // byteLength
            result = parseMemberInteger(object, "byteLength", buffer.byteLength);
            if (result.failed()) return ERROR + result;

            // [name]
            result = parseMemberStringOptional(object, "name", buffer.name);
            if (result.failed()) return ERROR + result;

            return SUCCESS;
        }

        GlTFParseResult parseBufferView(const JsonObject& object, GlTFBufferView& bufferView) {
            const auto ERROR = GlTFParseResult::error("parseBufferView");
            GlTFParseResult result;

            // buffer
            result = parseMemberInteger(object, "buffer", bufferView.buffer);
            if (result.failed()) return ERROR + result;

            // [byteOffset]
            result = parseMemberIntegerOptional(object, "byteOffset", bufferView.byteOffset);
            if (result.failed()) return ERROR + result;

            // byteLength
            result = parseMemberInteger(object, "byteLength", bufferView.byteLength);
            if (result.failed()) return ERROR + result;

            // [byteStride]
            result = parseMemberIntegerOptional(object, "byteStride", bufferView.byteStride);
            if (result.failed()) return ERROR + result;

            // [target]
            result = parseMemberIntegerOptional(object, "target", bufferView.target);
            if (result.failed()) return ERROR + result;

            // [name]
            result = parseMemberStringOptional(object, "name", bufferView.name);
            if (result.failed()) return ERROR + result;

            return SUCCESS;
        }

        GlTFParseResult parseNode(const JsonObject& object, GlTFNode& node) {
            const auto ERROR = GlTFParseResult::error("parseNode");
            JsonParseResult json_result;
            GlTFParseResult result;

            // [camera]
            result = parseMemberIntegerOptional(object, "camera", node.camera);
            if (result.failed()) return ERROR + result;

            // [children]
            result = parseMemberIntegerArrayOptional(object, "children", node.children);
            if (result.failed()) return ERROR + result;

            // [skin]
            result = parseMemberIntegerOptional(object, "skin", node.skin);
            if (result.failed()) return ERROR + result;

            // [matrix]
            auto* p_matrix = getJsonMemberArray(object, JsonString("matrix"), json_result, true);
            if (json_result.failed()) return ERROR + GlTFParseResult(json_result);
            if (p_matrix != nullptr) {
                Matrix4x4f matrix;
                result = parseMatrix4(*p_matrix, matrix);
                if (result.failed()) return ERROR + result;

                node.matrix = makeOptional<Matrix4x4f>(base::move(matrix));
            } else {
                node.matrix = Optional<Matrix4x4f>();
            }

            // [mesh]
            result = parseMemberIntegerOptional(object, "mesh", node.mesh);
            if (result.failed()) return ERROR + result;

            // [rotation]
            auto* p_rotation =
                getJsonMemberArray(object, JsonString("rotation"), json_result, true);
            if (json_result.failed()) return ERROR + GlTFParseResult(json_result);
            if (p_rotation != nullptr) {
                Vector4f rotation;
                result = parseVector4(*p_rotation, rotation);
                if (result.failed()) return ERROR + result;

                node.rotation = makeOptional<Vector4f>(base::move(rotation));
            } else {
                node.rotation = Optional<Vector4f>();
            }

            // [scale]
            auto* p_scale = getJsonMemberArray(object, JsonString("scale"), json_result, true);
            if (json_result.failed()) return ERROR + GlTFParseResult(json_result);
            if (p_scale != nullptr) {
                Vector3f scale;
                result = parseVector3(*p_scale, scale);
                if (result.failed()) return ERROR + result;

                node.scale = makeOptional<Vector3f>(base::move(scale));
            } else {
                node.scale = Optional<Vector3f>();
            }

            // [translation]
            auto* p_translation =
                getJsonMemberArray(object, JsonString("translation"), json_result, true);
            if (json_result.failed()) return ERROR + GlTFParseResult(json_result);
            if (p_translation != nullptr) {
                Vector3f translation;
                result = parseVector3(*p_translation, translation);
                if (result.failed()) return ERROR + result;

                node.translation = makeOptional<Vector3f>(base::move(translation));
            } else {
                node.translation = Optional<Vector3f>();
            }

            // [name]
            result = parseMemberStringOptional(object, "name", node.name);
            if (result.failed()) return ERROR + result;

            return SUCCESS;
        }

        GlTFParseResult parseScene(const JsonObject& object, GlTFScene& scene) {
            const auto ERROR = GlTFParseResult::error("parseScene");
            GlTFParseResult result;

            // [nodes]
            result = parseMemberIntegerArrayOptional(object, "nodes", scene.nodes);
            if (result.failed()) return ERROR + result;

            // [name]
            result = parseMemberStringOptional(object, "name", scene.name);
            if (result.failed()) return ERROR + result;

            return SUCCESS;
        }

    }  // namespace

    GlTFParseResult parseGlTF(const JsonValue& json, GlTF& gltf) {
        if (json.type != JsonValueType::object) return EXPECTED_OBJECT;
        auto& top = json.object;

        JsonParseResult json_result;
        GlTFParseResult result;

        // asset
        auto* p_asset = getJsonMemberObject(top, JsonString("asset"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        result = parseAsset(*p_asset, gltf.asset);
        if (result.failed()) return result;

        // [buffers]
        auto* p_buffers = getJsonMemberArray(top, JsonString("buffers"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_buffers != nullptr) {
            vector<GlTFBuffer> buffers;
            for (auto& element : p_buffers->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFBuffer buffer;
                result = parseBuffer(element.object, buffer);
                if (result.failed()) return result;

                buffers.push(base::move(buffer));
            }

            gltf.buffers = makeOptional<vector<GlTFBuffer>>(base::move(buffers));
        } else {
            gltf.buffers = Optional<vector<GlTFBuffer>>();
        }

        // [bufferViews]
        auto* p_bufferViews = getJsonMemberArray(top, JsonString("bufferViews"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_bufferViews != nullptr) {
            vector<GlTFBufferView> bufferViews;
            for (auto& element : p_bufferViews->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFBufferView bufferView;
                result = parseBufferView(element.object, bufferView);
                if (result.failed()) return result;

                bufferViews.push(base::move(bufferView));
            }

            gltf.bufferViews = makeOptional<vector<GlTFBufferView>>(base::move(bufferViews));
        } else {
            gltf.bufferViews = Optional<vector<GlTFBufferView>>();
        }

        // [nodes]
        auto* p_nodes = getJsonMemberArray(top, JsonString("nodes"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_nodes != nullptr) {
            vector<GlTFNode> nodes;
            for (auto& element : p_nodes->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFNode node;
                result = parseNode(element.object, node);
                if (result.failed()) return result;

                nodes.push(base::move(node));
            }

            gltf.nodes = makeOptional<vector<GlTFNode>>(base::move(nodes));
        } else {
            gltf.nodes = Optional<vector<GlTFNode>>();
        }

        // [scene]
        result = parseMemberIntegerOptional(top, "scene", gltf.scene);
        if (result.failed()) return result;

        // [scenes]
        auto* p_scenes = getJsonMemberArray(top, JsonString("scenes"), json_result, true);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_scenes != nullptr) {
            vector<GlTFScene> scenes;
            for (auto& element : p_scenes->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFScene scene;
                result = parseScene(element.object, scene);
                if (result.failed()) return result;

                scenes.push(base::move(scene));
            }

            gltf.scenes = makeOptional<vector<GlTFScene>>(base::move(scenes));
        } else {
            gltf.scenes = Optional<vector<GlTFScene>>();
        }

        return SUCCESS;
    }

}  // namespace spargel::codec::model
