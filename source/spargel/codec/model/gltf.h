/*
 * A glTF 2.0 parser implementation
 *
 * This is base on the specification document *glTF 2.0 Specification* (Version 2.0.1)
 *
 * Some of the comments are taken and adapted from this document.
 */

#pragma once

#include <spargel/base/optional.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>
#include <spargel/codec/json.h>
#include <spargel/math/matrix.h>
#include <spargel/math/vector.h>

namespace spargel::codec::model {

    using base::Optional;
    using base::vector;

    using math::Matrix4x4f;
    using math::Vector3f;
    using math::Vector4f;

    using GlTFInteger = int;
    using GlTFNumber = double;
    using GlTFString = base::string;

    /*
     * Metadata about the glTF asset.
     */
    struct GlTFAsset {
        // A copyright message suitable for display to credit the content creator.
        Optional<GlTFString> copyright;

        // Tool that generated this glTF model. Useful for debugging.
        Optional<GlTFString> generator;

        // The glTF version in the form of <major>.<minor> that this asset targets.
        // Pattern: ^[0-9]+\.[0-9]+$
        GlTFString version;

        // The minimum glTF version in the form of <major>.<minor> that this asset targets. This
        // property MUST NOT be greater than the asset version.
        // Pattern: ^[0-9]+\.[0-9]+$
        Optional<GlTFString> minVersion;
    };

    /*
     * A buffer points to binary geometry, animation, or skins.
     */
    struct GlTFBuffer {
        // The URI (or IRI) of the buffer. Relative paths are relative to the current glTF asset.
        // Instead of referencing an external file, this field MAY contain a data:-URI.
        // Format: iri-reference.
        Optional<GlTFString> uri;

        // The length of the buffer in bytes. (>= 1)
        GlTFInteger byteLength;

        // The user-defined name of this object. This is not necessarily unique, e.g., an accessor
        // and a buffer could have the same name, or two accessors could even have the same name.
        Optional<GlTFString> name;
    };

    /*
     * A view into a buffer generally representing a subset of the buffer.
     */
    struct GlTFBufferView {
        // The index of the buffer. (>=0)
        GlTFInteger buffer;

        // The offset into the buffer in bytes. (>= 0)
        // Default: 0.
        Optional<GlTFInteger> byteOffset;

        // The length of the bufferView in bytes. (>= 1)
        GlTFInteger byteLength;

        // The stride, in bytes, between vertex attributes. When this is not defined, data is
        // tightly packed. When two or more accessors use the same buffer view, this field MUST be
        // defined. (>= 4, <= 252)
        Optional<GlTFInteger> byteStride;

        // The hint representing the intended GPU buffer type to use with this buffer view.
        // Allowed: 34962=ARRAY_BUFFER, 34963=ELEMENT_ARRAY_BUFFER.
        Optional<GlTFInteger> target;

        // The user-defined name of this object. This is not necessarily unique, e.g., an accessor
        // and a buffer could have the same name, or two accessors could even have the same name.
        Optional<GlTFString> name;
    };

    /*
     * A node in the node hierarchy.
     */
    struct GlTFNode {
        // The index of the camera referenced by this node. (>=0)
        Optional<GlTFInteger> camera;

        // The indices of this node’s children. (unique, >= 0)
        Optional<vector<GlTFInteger>> children;

        // The index of the skin referenced by this node. When a skin is referenced by a node within
        // a scene, all joints used by the skin MUST belong to the same scene. When defined, mesh
        // MUST also be defined. (>=0)
        Optional<GlTFInteger> skin;

        // A floating-point 4x4 transformation matrix stored in column-major order.
        // Default: [1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1].
        Optional<Matrix4x4f> matrix;

        // The index of the mesh in this node. (>= 0)
        Optional<GlTFInteger> mesh;

        // The node’s unit quaternion rotation in the order (x, y, z, w), where w is the scalar.
        // Default:[0,0,0,1].
        Optional<Vector4f> rotation;

        // The node’s non-uniform scale, given as the scaling factors along the x, y, and z axes.
        // Default: [1,1,1].
        Optional<Vector3f> scale;

        // The node’s translation along the x, y, and z axes.
        // Default: [0,0,0].
        Optional<Vector3f> translation;

        // The weights of the instantiated morph target. The number of array elements MUST match the
        // number of morph targets of the referenced mesh. When defined, mesh MUST also be defined.
        Optional<vector<GlTFNumber>> weights;

        // The user-defined name of this object. This is not necessarily unique, e.g., an accessor
        // and a buffer could have the same name, or two accessors could even have the same name.
        Optional<GlTFString> name;
    };

    /*
     * The root nodes of a scene.
     */
    struct GlTFScene {
        // The indices of each root node. (unique, >= 0)
        Optional<vector<GlTFInteger>> nodes;

        // The user-defined name of this object. This is not necessarily unique, e.g., an accessor
        // and a buffer could have the same name, or two accessors could even have the same name.
        Optional<GlTFString> name;
    };

    /*
     * The root object for a glTF asset.
     */
    struct GlTF {
        // Metadata about the glTF asset.
        GlTFAsset asset;

        // An array of buffers. A buffer points to binary geometry, animation, or skins.
        Optional<vector<GlTFBuffer>> buffers;

        // An array of bufferViews. A bufferView is a view into a buffer generally representing a
        // subset of the buffer.
        Optional<vector<GlTFBufferView>> bufferViews;

        // An array of nodes.
        Optional<vector<GlTFNode>> nodes;

        // The index of the default scene. This property MUST NOT be defined, when scenes is
        // undefined. (>= 0)
        Optional<GlTFInteger> scene;

        // An array of scenes.
        Optional<vector<GlTFScene>> scenes;
    };

    class GlTFParseResult {
    public:
        GlTFParseResult() : _failed(false), _msg("") {}

        explicit GlTFParseResult(const JsonParseResult& jsonResult)
            : _failed(jsonResult.failed()), _msg(jsonResult.message()) {}

        bool failed() const { return _failed; }

        base::string const& message() const { return _msg; }

        friend GlTFParseResult operator+(const GlTFParseResult& result1,
                                         const GlTFParseResult& result2);

        static GlTFParseResult success() { return GlTFParseResult(false, base::string()); }

        static GlTFParseResult error(const base::string& msg) { return GlTFParseResult(true, msg); }

        static GlTFParseResult error(const char* msg = "failed to parse glTF") {
            return error(base::string(msg));
        }

    private:
        GlTFParseResult(bool failed, const base::string& msg) : _failed(failed), _msg(msg) {}

        bool _failed;
        base::string _msg;
    };

    GlTFParseResult parseGlTF(const JsonValue& json, GlTF& gltf);

}  // namespace spargel::codec::model
