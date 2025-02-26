#include <spargel/codec/model/gltf.h>

namespace spargel::codec::model {

    namespace {

        const GlTFParseResult SUCCESS = GlTFParseResult::success();
        const GlTFParseResult EXPECTED_OBJECT = GlTFParseResult::error("expected an object");

        GlTFParseResult parseNonNegativeInteger(const JsonNumber& number, u32& v) {
            v = u32(number);
            return number == v ? SUCCESS
                               : GlTFParseResult::error("expected a non-negative integer");
        }

        GlTFParseResult parseNonNegativeInteger(const JsonValue& value, u32& v) {
            if (value.type != JsonValueType::number)
                return GlTFParseResult::error("expected a number");

            return parseNonNegativeInteger(value.number, v);
        }

        GlTFParseResult parseGlTfAsset(JsonObject& json, GlTFAsset& asset) {
            JsonParseResult json_result;

            // version
            auto* p_version = getJsonMemberString(json, JsonString("version"), json_result);
            if (json_result.failed()) return GlTFParseResult(json_result);
            asset.version = *p_version;

            // [minVersion]
            auto* p_minVersion =
                getJsonMemberString(json, JsonString("minVersion"), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);
            asset.minVersion = p_minVersion == nullptr
                                   ? base::Optional<base::string>()
                                   : base::makeOptional<base::string>(*p_minVersion);

            return SUCCESS;
        }

        GlTFParseResult parseGlTFScene(JsonObject& json, GlTFScene& scene) {
            JsonParseResult json_result;
            GlTFParseResult result;

            // [name]
            auto* p_name = getJsonMemberString(json, JsonString("name"), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);
            scene.name = p_name == nullptr ? base::Optional<base::string>()
                                           : base::makeOptional<base::string>(*p_name);

            // nodes
            auto* p_nodes = getJsonMemberArray(json, JsonString("nodes"), json_result);
            if (json_result.failed()) return GlTFParseResult(json_result);

            for (auto& element : p_nodes->elements) {
                u32 v;
                result = parseNonNegativeInteger(element, v);
                if (result.failed()) return result;

                scene.nodes.push(v);
            }

            return SUCCESS;
        }

        GlTFParseResult parseMatrix(JsonArray& array, math::Matrix4x4f& matrix) {
            if (array.elements.count() != 16)
                return GlTFParseResult::error("expected matrix entries to be 16");

            for (int i = 0; i < 16; i++) {
                auto& element = array.elements[i];
                if (element.type != JsonValueType::number)
                    return GlTFParseResult::error("expected a number");

                matrix.entries[i] = (float)element.number;
            }

            return SUCCESS;
        }

        GlTFParseResult parseGlTFNode(JsonObject& json, GlTFNode& node) {
            JsonParseResult json_result;
            GlTFParseResult result;

            // [name]
            auto* p_name = getJsonMemberString(json, JsonString("name"), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);
            node.name = p_name == nullptr ? base::Optional<base::string>()
                                          : base::makeOptional<base::string>(*p_name);

            // [children]
            auto* p_children = getJsonMemberArray(json, JsonString("children"), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);
            if (p_children != nullptr) {
                base::vector<u32> children;
                for (auto& element : p_children->elements) {
                    u32 v;
                    result = parseNonNegativeInteger(element, v);
                    if (result.failed()) return result;

                    children.push(v);
                }
                node.children = base::makeOptional<base::vector<u32>>(base::move(children));
            } else {
                node.children = base::Optional<base::vector<u32>>();
            }

            // [matrix]
            auto* p_matrix = getJsonMemberArray(json, JsonString("matrix"), json_result, true);
            if (json_result.failed()) return GlTFParseResult(json_result);
            if (p_matrix != nullptr) {
                math::Matrix4x4f matrix;
                result = parseMatrix(*p_matrix, matrix);
                if (result.failed()) return result;

                node.matrix = base::makeOptional<math::Matrix4x4f>(base::move(matrix));
            } else {
                node.matrix = base::Optional<math::Matrix4x4f>();
            }

            return SUCCESS;
        }

    }  // namespace

    GlTFParseResult parseGlTF(JsonValue& json, GlTFObject& gltf) {
        if (json.type != JsonValueType::object) return EXPECTED_OBJECT;
        auto& top = json.object;

        JsonParseResult json_result;
        GlTFParseResult result;

        // asset
        auto* p_asset = getJsonMemberObject(top, JsonString("asset"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        result = parseGlTfAsset(*p_asset, gltf.asset);
        if (result.failed()) return result;

        // [scenes]
        auto* p_scenes = getJsonMemberArray(top, JsonString("scenes"), json_result, true);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_scenes != nullptr) {
            base::vector<GlTFScene> scenes;
            for (auto& element : p_scenes->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFScene scene;
                result = parseGlTFScene(element.object, scene);
                if (result.failed()) return result;

                scenes.push(base::move(scene));
            }

            gltf.scenes = base::makeOptional<base::vector<GlTFScene>>(base::move(scenes));
        } else {
            gltf.scenes = base::Optional<base::vector<GlTFScene>>();
        }

        // [scene]
        auto* p_scene = getJsonMemberNumber(top, JsonString("scene"), json_result, true);
        if (json_result.failed()) return GlTFParseResult(json_result);

        if (p_scene != nullptr) {
            u32 v;
            result = parseNonNegativeInteger(*p_scene, v);
            if (result.failed()) return result;
            gltf.scene = base::makeOptional<u32>(v);
        } else {
            gltf.scene = base::Optional<u32>();
        }

        // [nodes]
        auto* p_nodes = getJsonMemberArray(top, JsonString("nodes"), json_result);
        if (json_result.failed()) return GlTFParseResult(json_result);
        if (p_nodes != nullptr) {
            base::vector<GlTFNode> nodes;
            for (auto& element : p_nodes->elements) {
                if (element.type != JsonValueType::object) return EXPECTED_OBJECT;

                GlTFNode node;
                result = parseGlTFNode(element.object, node);
                if (result.failed()) return result;

                nodes.push(base::move(node));
            }

            gltf.nodes = base::makeOptional<base::vector<GlTFNode>>(base::move(nodes));
        } else {
            gltf.nodes = base::Optional<base::vector<GlTFNode>>();
        }

        return SUCCESS;
    }

}  // namespace spargel::codec::model
