/*
 * A glTF 2.x parser implementation
 */

#pragma once

#include <spargel/base/optional.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>
#include <spargel/codec/json.h>
#include <spargel/math/matrix.h>

namespace spargel::codec::model {

    // "asset" property
    struct GlTFAsset {
        // target glTF version
        base::string version;

        // minimum glTF version support required
        base::Optional<base::string> minVersion;
    };

    struct GlTFScene {
        base::Optional<base::string> name;

        base::vector<u32> nodes;
    };

    struct GlTFNode {
        base::Optional<base::string> name;

        base::Optional<base::vector<u32>> children;

        base::Optional<math::Matrix4x4f> matrix;
    };

    // the top glTF object
    struct GlTFObject {
        GlTFAsset asset;

        base::Optional<base::vector<GlTFScene>> scenes;

        base::Optional<u32> scene;

        base::Optional<base::vector<GlTFNode>> nodes;
    };

    class GlTFParseResult {
    public:
        GlTFParseResult() : _failed(false), _msg("") {}

        explicit GlTFParseResult(const JsonParseResult& jsonResult)
            : _failed(jsonResult.failed()), _msg(jsonResult.message()) {}

        bool failed() const { return _failed; }

        static GlTFParseResult success() { return GlTFParseResult(false, base::string()); }

        static GlTFParseResult error(const base::string& msg) { return GlTFParseResult(true, msg); }

        static GlTFParseResult error(const char* msg = "failed to parse GlTF") {
            return error(base::string(msg));
        }

        base::string const& message() const { return _msg; }

    private:
        GlTFParseResult(bool failed, const base::string& msg) : _failed(failed), _msg(msg) {}

        bool _failed;
        base::string _msg;
    };

    GlTFParseResult parseGlTF(JsonValue& json, GlTFObject& gltf);

}  // namespace spargel::codec::model
