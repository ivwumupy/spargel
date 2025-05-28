#include <spargel/codec/codec2.h>
#include <spargel/codec/json.h>
#include <spargel/codec/model/gltf.h>

namespace spargel::codec::model {

    namespace {

        using namespace spargel::base::literals;

        using DB = JsonDecodeBackend;

        template <DecodeBackend DB>
        class Vector3fDecoder : public Decoder2<DB, Vector3f> {
        public:
            base::Either<Vector3f, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 3) {
                        Vector3f v;
                        v.x = array[0];
                        v.y = array[1];
                        v.z = array[2];
                        return base::Left(base::move(v));
                    } else {
                        return base::Right(typename DB::ErrorType("Vector3f expected 3 numbers"_sv));
                    }
                } else {
                    return base::Right(base::move(result.right()));
                }
            }

            base::unique_ptr<Decoder2<DB, Vector3f>> clone() const override {
                return base::make_unique<Vector3fDecoder<DB>>();
            }
        };

        template <DecodeBackend DB>
        class Vector4fDecoder : public Decoder2<DB, Vector4f> {
        public:
            base::Either<Vector4f, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 4) {
                        Vector4f v;
                        v.x = array[0];
                        v.y = array[1];
                        v.z = array[2];
                        v.w = array[3];
                        return base::Left(base::move(v));
                    } else {
                        return base::Right(typename DB::ErrorType("Vector4f expected 4 numbers"_sv));
                    }
                } else {
                    return base::Right(base::move(result.right()));
                }
            }

            base::unique_ptr<Decoder2<DB, Vector4f>> clone() const override {
                return base::make_unique<Vector4fDecoder<DB>>();
            }
        };

        template <DecodeBackend DB>
        class Matrix4x4fDecoder : public Decoder2<DB, Matrix4x4f> {
        public:
            base::Either<Matrix4x4f, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
                auto result = CodecArray<CodecF64>::decode(backend, data);
                if (result.isLeft()) {
                    auto array = result.left();
                    if (array.count() == 16) {
                        Matrix4x4f mat;
                        for (int i = 0; i < 16; i++) mat.entries[i] = array[i];
                        return base::Left(base::move(mat));
                    } else {
                        return base::Right(typename DB::ErrorType("Matrix4x4f expected 16 numbers"_sv));
                    }
                } else {
                    return base::Right(base::move(result.right()));
                }
            }

            base::unique_ptr<Decoder2<DB, Matrix4x4f>> clone() const override {
                return base::make_unique<Matrix4x4fDecoder<DB>>();
            }
        };

        auto glTFAccessorDecoder = makeRecordDecoder<DB, GlTFAccessor>(
            base::Constructor<GlTFAccessor>{},
            I32Decoder<DB>().optionalFieldOf("bufferView"_sv),
            I32Decoder<DB>().optionalFieldOf("byteOffset"_sv),
            I32Decoder<DB>().fieldOf("componentType"_sv),
            BooleanDecoder<DB>().optionalFieldOf("normalized"_sv),
            I32Decoder<DB>().fieldOf("count"_sv),
            StringDecoder<DB>().fieldOf("type"_sv),
            F64Decoder<DB>().arrayOf().optionalFieldOf("max"_sv),
            F64Decoder<DB>().arrayOf().optionalFieldOf("min"_sv),
            StringDecoder<DB>().optionalFieldOf("name"_sv));

        auto glTFAssetDecoder = makeRecordDecoder<DB, GlTFAsset>(
            base::Constructor<GlTFAsset>{},
            StringDecoder<DB>().optionalFieldOf("copyright"_sv),
            StringDecoder<DB>().optionalFieldOf("generator"_sv),
            StringDecoder<DB>().fieldOf("version"_sv),
            StringDecoder<DB>().optionalFieldOf("minVersion"_sv));

        auto glTFBufferDecoder = makeRecordDecoder<DB, GlTFBuffer>(
            base::Constructor<GlTFBuffer>{},
            StringDecoder<DB>().optionalFieldOf("uri"_sv),
            I32Decoder<DB>().fieldOf("byteLength"_sv),
            StringDecoder<DB>().optionalFieldOf("name"_sv));

        auto glTFBufferViewDecoder = makeRecordDecoder<DB, GlTFBufferView>(
            base::Constructor<GlTFBufferView>{},
            I32Decoder<DB>().fieldOf("buffer"_sv),
            I32Decoder<DB>().optionalFieldOf("byteOffset"_sv),
            I32Decoder<DB>().fieldOf("byteLength"_sv),
            I32Decoder<DB>().optionalFieldOf("byteStride"_sv),
            I32Decoder<DB>().optionalFieldOf("target"_sv),
            StringDecoder<DB>().optionalFieldOf("name"_sv));

        auto glTFNodeDecoder = makeRecordDecoder<DB, GlTFNode>(
            base::Constructor<GlTFNode>{},
            I32Decoder<DB>().optionalFieldOf("camera"_sv),
            I32Decoder<DB>().arrayOf().optionalFieldOf("children"_sv),
            I32Decoder<DB>().optionalFieldOf("skin"_sv),
            Matrix4x4fDecoder<DB>().optionalFieldOf("matrix"_sv),
            I32Decoder<DB>().optionalFieldOf("mesh"_sv),
            Vector4fDecoder<DB>().optionalFieldOf("rotation"_sv),
            Vector3fDecoder<DB>().optionalFieldOf("scale"_sv),
            Vector3fDecoder<DB>().optionalFieldOf("translation"_sv),
            F64Decoder<DB>().arrayOf().optionalFieldOf("weights"_sv),
            StringDecoder<DB>().optionalFieldOf("name"_sv));

        auto glTFSceneDecoder = makeRecordDecoder<DB, GlTFScene>(
            base::Constructor<GlTFScene>{},
            I32Decoder<DB>().arrayOf().optionalFieldOf("nodes"_sv),
            StringDecoder<DB>().optionalFieldOf("name"_sv));

        auto glTFDecoder = makeRecordDecoder<DB, GlTF>(
            base::Constructor<GlTF>{},
            glTFAccessorDecoder.arrayOf().optionalFieldOf("accessors"_sv),
            glTFAssetDecoder.fieldOf("asset"_sv),
            glTFBufferDecoder.arrayOf().optionalFieldOf("buffers"_sv),
            glTFBufferViewDecoder.arrayOf().optionalFieldOf("bufferViews"_sv),
            glTFNodeDecoder.arrayOf().optionalFieldOf("nodes"_sv),
            I32Decoder<DB>().optionalFieldOf("scene"_sv),
            glTFSceneDecoder.arrayOf().optionalFieldOf("scenes"_sv));

    }  // namespace

    base::Either<GlTF, GlTFDecodeError> parseGlTF(const char* text, usize len) {
        auto json_result = parseJson(text, len);
        if (json_result.isRight())
            return base::makeRight<GlTF, GlTFDecodeError>(json_result.right().message());

        JsonDecodeBackend backend;
        auto result = glTFDecoder.decode(backend, base::move(json_result.left()));
        if (result.isLeft())
            return base::Left(base::move(result.left()));
        else
            return base::Right(GlTFDecodeError(base::move(result.right().message())));
    }

}  // namespace spargel::codec::model
