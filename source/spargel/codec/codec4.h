#pragma once

#include <spargel/codec/codec.h>

namespace spargel::codec {

    template <typename B /* codec backend type */>
    concept CodecBackend = requires {
        typename B::EncodeBackendType;
        typename B::DecodeBackendType;
    };

    template <EncodeBackend EB, DecodeBackend DB>
    struct CodecBackendOf {
        using EncodeBackendType = EB;
        using DecodeBackendType = DB;
    };

    template <typename C>
    concept Encoder4 = requires {
        typename C::TargetType;
    } && requires(C& codec, DummyEncodeBackend& backend, const C::TargetType& object) {
        { codec.encode(backend, object) } -> base::SameAs<base::Either<DummyType, CodecError>>;
    };

    template <typename C>
    concept Decoder4 = requires {
        typename C::TargetType;
    } && requires(C& codec, DummyDecodeBackend& backend, const DummyType& data) {
        { codec.decode(backend, data) } -> base::SameAs<base::Either<typename C::TargetType, CodecError>>;
    };

    template <typename C>
    concept Codec = Encoder4<C> && Decoder4<C>;

    template <typename T>
    class ErrorEncoder {
    public:
        using TargetType = T;

        ErrorEncoder(const base::string& message) : _message(message) {}
        ErrorEncoder(base::string_view message) : _message(message) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) {
            return base::Right(typename EB::ErrorType(_message));
        }

    private:
        base::string _message;
    };

    template <typename T>
    class ErrorDecoder {
    public:
        using TargetType = T;

        ErrorDecoder(const base::string& message) : _message(message) {}
        ErrorDecoder(base::string_view message) : _message(message) {}

        template <DecodeBackend DB>
        base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) {
            return base::Right(typename DB::ErrorType(_message));
        }

    private:
        base::string _message;
    };

    template <typename T>
    class ErrorCodec : public ErrorEncoder<T>, public ErrorDecoder<T> {
    public:
        using TargetType = T;

        ErrorCodec(const base::string& encode_message, const base::string& decode_message)
            : ErrorEncoder<T>(encode_message), ErrorDecoder<T>(decode_message) {}
        ErrorCodec(base::string_view encode_message, base::string_view decode_message)
            : ErrorEncoder<T>(encode_message), ErrorDecoder<T>(decode_message) {}
    };

    struct NullCodec {
        using TargetType = nullptr_t;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, base::nullptr_t ptr = nullptr) {
            return backend.makeNull();
        }
        template <DecodeBackend DB>
        base::Either<base::nullptr_t, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            auto result = backend.getNull(data);
            if (!result.hasValue()) {
                return base::Left(nullptr);
            } else {
                return base::Right(base::move(result.value()));
            }
        }
    };

    struct BooleanCodec {
        using TargetType = bool;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, bool b) {
            return backend.makeBoolean(b);
        }

        template <DecodeBackend DB>
        base::Either<bool, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getBoolean(data);
        }
    };

    struct U32Codec {
        using TargetType = u32;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, u32 n) {
            return backend.makeU32(n);
        }

        template <DecodeBackend DB>
        base::Either<u32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getU32(data);
        }
    };

    struct I32Codec {
        using TargetType = i32;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, i32 n) {
            return backend.makeI32(n);
        }

        template <DecodeBackend DB>
        base::Either<i32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getI32(data);
        }
    };

    struct F32Codec {
        using TargetType = f32;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, f32 x) {
            return backend.makeF32(x);
        }

        template <DecodeBackend DB>
        base::Either<f32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getF32(data);
        }
    };

    struct F64Codec {
        using TargetType = f64;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, f64 x) {
            return backend.makeF64(x);
        }

        template <DecodeBackend DB>
        base::Either<f64, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getF64(data);
        }
    };

    struct StringCodec {
        using TargetType = base::string;

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::string& s) {
            return backend.makeString(s);
        }

        template <DecodeBackend DB>
        base::Either<base::string, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getString(data);
        }
    };

    namespace _vector {

        template <Encoder4 E, EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encodeVector(E& encoder, EB& backend, const base::vector<typename E::TargetType>& v) {
            base::vector<typename EB::DataType> array;
            for (auto& item : v) {
                auto result = encoder.encode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }
            return backend.makeArray(base::move(array));
        }

        template <Decoder4 D, DecodeBackend DB>
        base::Either<base::vector<typename D::TargetType>, typename DB::ErrorType> decode(D& decoder, DB& backend, const typename DB::DataType& data) {
            auto data_array = backend.getArray(data);
            if (data_array.isRight())
                return base::Right(base::move(data_array.right()));

            base::vector<typename D::TargetType> array;
            for (auto& item : data_array.left()) {
                auto result = decoder.decode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }

            return base::Left(base::move(array));
        }

    }  // namespace _vector

    template <Encoder4 E>
    class VectorEncoder {
        using T = typename E::TargetType;

    public:
        using TargetType = base::vector<T>;

        VectorEncoder(const E& encoder) : _encoder(encoder) {}
        VectorEncoder(E&& encoder) : _encoder(base::move(encoder)) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::vector<T>& v) {
            return _vector::encodeVector(_encoder, backend, v);
        }

    private:
        E _encoder;
    };

    template <Encoder4 E>
    VectorEncoder<E> makeVectorEncoder(const E& encoder) { return VectorEncoder(encoder); }
    template <Encoder4 E>
    VectorEncoder<E> makeVectorEncoder(E&& encoder) { return VectorEncoder(base::move(encoder)); }

    template <Decoder4 D>
    class VectorDecoder {
        using T = typename D::TargetType;

    public:
        using TargetType = base::vector<T>;

        VectorDecoder(const D& decoder) : _decoder(decoder) {}
        VectorDecoder(D&& decoder) : _decoder(base::move(decoder)) {}

        template <DecodeBackend DB>
        base::Either<base::vector<T>, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _vector::decode(_decoder, backend, data);
        }

    private:
        D _decoder;
    };

    template <Decoder4 D>
    VectorDecoder<D> makeVectorDecoder(const D& decoder) { return VectorDecoder(decoder); }
    template <Decoder4 D>
    VectorDecoder<D> makeVectorDecoder(D&& decoder) { return VectorDecoder(base::move(decoder)); }

    template <Codec C>
    class VectorCodec {
        using T = typename C::TargetType;

    public:
        using TargetType = base::vector<T>;

        VectorCodec(const C& codec) : _codec(codec) {}
        VectorCodec(C&& codec) : _codec(base::move(codec)) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::vector<T>& v) {
            return _vector::encodeVector(_codec, backend, v);
        }

        template <DecodeBackend DB>
        base::Either<base::vector<T>, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _vector::decode(_codec, backend, data);
        }

    private:
        C _codec;
    };

    template <Codec C>
    VectorCodec<C> makeVectorCodec(const C& codec) { return VectorCodec(codec); }
    template <Codec C>
    VectorCodec<C> makeVectorCodec(C&& codec) { return VectorCodec(base::move(codec)); }

    template <typename B>
    concept RecordBuilder = requires {
        typename B::BackendType;
    } && EncodeBackend<typename B::BackendType> && requires(B& builder, const base::string& name, const B::BackendType::DataType& value) {
        builder.add(name, value);
    } && requires(B& builder, const base::string& name, B::BackendType::DataType&& value) {
        builder.add(name, base::move(value));
    } && requires(B& builder, B::BackendType& backend) {
        {
            builder.build(backend)
        } -> base::SameAs<base::Either<typename B::BackendType::DataType, typename B::BackendType::ErrorType>>;
    };

    template <EncodeBackend EB>
    class RecordBuilderImpl {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        using BackendType = EB;

        RecordBuilderImpl() {}

        void add(const base::string& name, const DataType& value) {
            _map.set(name, value);
        }
        void add(const base::string& name, DataType&& value) {
            _map.set(name, base::move(value));
        }

        base::Either<DataType, ErrorType> build(EB& backend) {
            return backend.makeMap(_map);
        }

    private:
        base::HashMap<base::string, DataType> _map;
    };

    template <typename E>
    concept FieldEncoder = requires {
        typename E::TargetType;
    } && requires(E& encoder, DummyEncodeBackend& backend, RecordBuilderImpl<DummyEncodeBackend>& builder, const E::TargetType& object) {
        { encoder.encode(backend, builder, object) } -> base::SameAs<base::Optional<CodecError>>;
    };

    template <typename D>
    concept FieldDecoder = requires {
        typename D::TargetType;
    } && requires(D& decoder, DummyDecodeBackend& backend, const DummyType& data) {
        { decoder.decode(backend, data) } -> base::SameAs<base::Either<typename D::TargetType, CodecError>>;
    };

    template <Encoder4 E>
    class NormalFieldEncoder {
        using T = E::TargetType;

    public:
        using TargetType = T;

        NormalFieldEncoder(base::string_view name, const E& encoder) : _name(name), _encoder(encoder) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const T& object) {
            auto result = _encoder->encode(backend, object);
            if (result.isLeft()) {
                builder.add(_name, base::move(result.left()));
                return base::nullopt;
            } else {
                return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
            }
        }

    private:
        base::string _name;
        E _encoder;
    };

    template <Encoder4 E>
    class OptionalFieldEncoder {
        using T = E::TargetType;

    public:
        using TargetType = T;

        OptionalFieldEncoder(base::string_view name, const E& encoder) : _name(name), _encoder(encoder) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const T& object) {
            if (object.hasValue()) {
                auto result = _encoder->encode(backend, object.value());
                if (result.isRight()) {
                    return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
                } else {
                    builder.add(_name, base::move(result.left()));
                    return base::nullopt;
                }
            } else {
                return base::nullopt;
            }
        }

    private:
        base::string _name;
        E _encoder;
    };

}  // namespace spargel::codec
