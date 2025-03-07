#include <spargel/codec/json.h>
#include <spargel/codec/model/gltf.h>

namespace spargel::codec::model {

    namespace {

        struct CodecVector3f {
            using Type = Vector3f;

            template <DecodeBackend DB>
            static base::Either<Vector3f, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 3) {
                        Vector3f v;
                        v.x = array[0];
                        v.y = array[1];
                        v.z = array[2];
                        return base::makeLeft<Vector3f, typename DB::ErrorType>(base::move(v));
                    } else {
                        return base::makeRight<Vector3f, typename DB::ErrorType>(
                            "Vector3f expected 3 numbers");
                    }
                } else {
                    return base::makeRight<Vector3f, typename DB::ErrorType>(result.right());
                }
            }
        };
        static_assert(Decoder<CodecVector3f>);

        struct CodecVector4f {
            using Type = Vector4f;

            template <DecodeBackend DB>
            static base::Either<Vector4f, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 4) {
                        Vector4f v;
                        v.x = array[0];
                        v.y = array[1];
                        v.z = array[2];
                        v.w = array[3];
                        return base::makeLeft<Vector4f, typename DB::ErrorType>(base::move(v));
                    } else {
                        return base::makeRight<Vector4f, typename DB::ErrorType>(
                            "Vector4f expected 4 numbers");
                    }
                } else {
                    return base::makeRight<Vector4f, typename DB::ErrorType>(result.right());
                }
            }
        };
        static_assert(Decoder<CodecVector4f>);

        struct CodecMatrix4x4f {
            using Type = Matrix4x4f;

            template <DecodeBackend DB>
            static base::Either<Matrix4x4f, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 16) {
                        Matrix4x4f mat;
                        for (int i = 0; i < 16; i++) mat.entries[i] = array[i];
                        return base::makeLeft<Matrix4x4f, typename DB::ErrorType>(base::move(mat));
                    } else {
                        return base::makeRight<Matrix4x4f, typename DB::ErrorType>(
                            "Matrix4x4f expected 16 numbers");
                    }
                } else {
                    return base::makeRight<Matrix4x4f, typename DB::ErrorType>(result.right());
                }
            }
        };
        static_assert(Decoder<CodecMatrix4x4f>);

        struct CodecGlTFAsset {
            using Type = GlTFAsset;

            using Constructor = base::Constructor<GlTFAsset(
                const Optional<GlTFString>&, const Optional<GlTFString>&, const GlTFString&,
                const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFAsset, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTFAsset>(Constructor(), backend, data,  //
                                            DecodeField::Optional<CodecString>("copyright"),
                                            DecodeField::Optional<CodecString>("generator"),
                                            DecodeField::Required<CodecString>("version"),
                                            DecodeField::Optional<CodecString>("minVersion"));
            }
        };
        static_assert(Decoder<CodecGlTFAsset>);

        struct CodecGlTFBuffer {
            using Type = GlTFBuffer;

            using Constructor = base::Constructor<GlTFBuffer(
                const Optional<GlTFString>&, GlTFInteger, const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFBuffer, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTFBuffer>(
                    Constructor(), backend, data,  //
                    DecodeField::Optional<CodecString>("uri"),
                    DecodeField::Required<CodecGreaterEqual<CodecI32, 1>>("byteLength"),
                    DecodeField::Optional<CodecString>("name"));
            }
        };
        static_assert(Decoder<CodecGlTFBuffer>);

        struct CodecGlTFBufferView {
            using Type = GlTFBufferView;

            using Constructor = base::Constructor<GlTFBufferView(
                GlTFInteger, const Optional<GlTFInteger>&, GlTFInteger,
                const Optional<GlTFInteger>&, const Optional<GlTFInteger>&,
                const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFBufferView, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTFBufferView>(
                    Constructor(), backend, data,  //
                    DecodeField::Required<CodecGreaterEqual<CodecI32, 0>>("buffer"),
                    DecodeField::Optional<CodecGreaterEqual<CodecI32, 0>>("byteOffset"),
                    DecodeField::Required<CodecGreaterEqual<CodecI32, 1>>("byteLength"),
                    DecodeField::Optional<CodecInRange<CodecI32, 4, 252>>("byteStride"),
                    DecodeField::Optional<CodecI32>("target"),
                    DecodeField::Optional<CodecString>("name"));
            }
        };
        static_assert(Decoder<CodecGlTFBufferView>);

        struct CodecGlTFNode {
            using Type = GlTFNode;

            using Constructor = base::Constructor<GlTFNode(
                const Optional<GlTFInteger>&, const Optional<vector<GlTFInteger>>&,
                const Optional<GlTFInteger>&, const Optional<Matrix4x4f>&,
                const Optional<GlTFInteger>&, const Optional<Vector4f>&, const Optional<Vector3f>&,
                const Optional<Vector3f>&, const Optional<vector<GlTFNumber>>&,
                const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFNode, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTFNode>(
                    Constructor(), backend, data,  //
                    DecodeField::Optional<CodecGreaterEqual<CodecI32, 0>>("camera"),
                    DecodeField::Optional<CodecArray<CodecGreaterEqual<CodecI32, 0>>>("children"),
                    DecodeField::Optional<CodecGreaterEqual<CodecI32, 0>>("skin"),
                    DecodeField::Optional<CodecMatrix4x4f>("matrix"),
                    DecodeField::Optional<CodecGreaterEqual<CodecI32, 0>>("mesh"),
                    DecodeField::Optional<CodecVector4f>("rotation"),
                    DecodeField::Optional<CodecVector3f>("scale"),
                    DecodeField::Optional<CodecVector3f>("translation"),
                    DecodeField::Optional<CodecArray<CodecF64>>("weights"),
                    DecodeField::Optional<CodecString>("name"));
            }
        };
        static_assert(Decoder<CodecGlTFNode>);

        struct CodecGlTFScene {
            using Type = GlTFScene;

            using Constructor = base::Constructor<GlTFScene(const Optional<vector<GlTFInteger>>&,
                                                            const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFScene, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTFScene>(
                    Constructor(), backend, data,  //
                    DecodeField::Optional<CodecArray<CodecGreaterEqual<CodecI32, 0>>>("nodes"),
                    DecodeField::Optional<CodecString>("name"));
            }
        };
        static_assert(Decoder<CodecGlTFScene>);

        struct CodecGlTF {
            using Type = GlTF;

            using Constructor = base::Constructor<GlTF(
                const GlTFAsset&, const Optional<vector<GlTFBuffer>>&,
                const Optional<vector<GlTFBufferView>>&, const Optional<vector<GlTFNode>>&,
                const Optional<GlTFInteger>&, const Optional<vector<GlTFScene>>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTF, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap2<GlTF>(
                    Constructor(), backend, data,  //
                    DecodeField::Required<CodecGlTFAsset>("asset"),
                    DecodeField::Optional<CodecArray<CodecGlTFBuffer>>("buffers"),
                    DecodeField::Optional<CodecArray<CodecGlTFBufferView>>("bufferViews"),
                    DecodeField::Optional<CodecArray<CodecGlTFNode>>("nodes"),
                    DecodeField::Optional<CodecGreaterEqual<CodecI32, 0>>("scene"),
                    DecodeField::Optional<CodecArray<CodecGlTFScene>>("scenes"));
            }
        };
        static_assert(Decoder<CodecGlTF>);

    }  // namespace

    base::Either<GlTF, GlTFDecodeError> parseGlTF(const char* text, usize len) {
        auto json_result = parseJson(text, len);
        if (json_result.isRight())
            return base::makeRight<GlTF, GlTFDecodeError>(json_result.right().message());

        DecodeBackendJson backend;
        auto result = CodecGlTF::decode(backend, base::move(json_result.left()));
        if (result.isLeft())
            return base::makeLeft<GlTF, GlTFDecodeError>(result.left());
        else
            return base::makeRight<GlTF, GlTFDecodeError>(result.right().message());
    }

}  // namespace spargel::codec::model
