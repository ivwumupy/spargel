#include <spargel/codec/json.h>
#include <spargel/codec/model/gltf.h>

namespace spargel::codec::model {

    namespace {

        struct CodecGlTFAsset {
            using Type = GlTFAsset;

            using Constructor = base::Constructor<GlTFAsset(
                const Optional<GlTFString>&, const Optional<GlTFString>&, const GlTFString&,
                const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFAsset, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap<GlTFAsset>(Constructor(), backend, data,  //
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
                return decodeMap<GlTFBuffer>(
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
                return decodeMap<GlTFBufferView>(
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

        struct CodecGlTFScene {
            using Type = GlTFScene;

            using Constructor = base::Constructor<GlTFScene(const Optional<vector<GlTFInteger>>&,
                                                            const Optional<GlTFString>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTFScene, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap<GlTFScene>(
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
                const Optional<vector<GlTFBufferView>>&, const Optional<GlTFInteger>&,
                const Optional<vector<GlTFScene>>&)>;

            template <DecodeBackend DB>
            static base::Either<GlTF, typename DB::ErrorType> decode(
                DB& backend, const typename DB::DataType& data) {
                return decodeMap<GlTF>(
                    Constructor(), backend, data,  //
                    DecodeField::Required<CodecGlTFAsset>("asset"),
                    DecodeField::Optional<CodecArray<CodecGlTFBuffer>>("buffers"),
                    DecodeField::Optional<CodecArray<CodecGlTFBufferView>>("bufferViews"),
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
