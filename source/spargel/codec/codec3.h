#pragma once

#include <spargel/codec/codec.h>

// FIXME
#include <tuple>

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

    // encoder prototype
    template <typename E>
    concept Encoder3 = requires {
        typename E::TargetType;
        typename E::BackendType;
    } && requires(E& encoder, E::BackendType& backend, const E::TargetType& object) {
        {
            encoder.encode(backend, object)
        } -> base::SameAs<base::Either<typename E::BackendType::DataType, typename E::BackendType::ErrorType>>;
    };

    // decoder prototype
    template <typename D>
    concept Decoder3 = requires {
        typename D::TargetType;
        typename D::BackendType;
    } && requires(D& decoder, D::BackendType& backend, const D::BackendType::DataType& data) {
        {
            decoder.decode(backend, data)
        } -> base::SameAs<base::Either<typename D::TargetType, typename D::BackendType::ErrorType>>;
    };

    template <typename C>
    concept Codec = requires {
        typename C::TargetType;
        typename C::BackendType;
    } && requires(C& codec, C::BackendType::EncodeBackendType& backend, const C::TargetType& object) {
        {
            codec.encode(backend, object)
        } -> base::SameAs<base::Either<typename C::BackendType::EncodeBackendType::DataType, typename C::BackendType::EncodeBackendType::ErrorType>>;
    } && requires(C& codec, C::BackendType::DecodeBackendType& backend, const C::BackendType::DecodeBackendType::DataType& data) {
        {
            codec.decode(backend, data)
        } -> base::SameAs<base::Either<typename C::TargetType, typename C::BackendType::DecodeBackendType::ErrorType>>;
    };

    template <typename C>
    class AsEncoder {
        using EB = C::BackendType::EncodeBackendType;

    public:
        using TargetType = C::TargetType;
        using BackendType = EB;

        AsEncoder(const C& codec) : _codec(codec) {}
        AsEncoder(C&& codec) : _codec(base::move(codec)) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const TargetType& object) {
            return _codec.encode(backend, object);
        }

    private:
        C _codec;
    };

    template <typename C>
    class AsDecoder {
        using DB = C::BackendType::DecodeBackendType;

    public:
        using TargetType = C::TargetType;
        using BackendType = DB;

        AsDecoder(const C& codec) : _codec(codec) {}
        AsDecoder(C&& codec) : _codec(base::move(codec)) {}

        base::Either<TargetType, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _codec.decode(backend, data);
        }

    private:
        C _codec;
    };

    template <EncodeBackend EB, typename T>
    class ErrorEncoder {
    public:
        using TargetType = T;
        using BackendType = EB;

        ErrorEncoder(const base::string& message) : _message(message) {}
        ErrorEncoder(base::string_view message) : _message(message) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) {
            return base::Right(typename EB::ErrorType(_message));
        }

    private:
        base::string _message;
    };

    template <DecodeBackend DB, typename T>
    class ErrorDecoder {
    public:
        using TargetType = T;
        using BackendType = DB;

        ErrorDecoder(const base::string& message) : _message(message) {}
        ErrorDecoder(base::string_view message) : _message(message) {}

        base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) {
            return base::Right(typename DB::ErrorType(_message));
        }

    private:
        base::string _message;
    };

    template <CodecBackend B, typename T>
    class ErrorCodec : public ErrorEncoder<typename B::EncodeBackendType, T>, public ErrorDecoder<typename B::DecodeBackendType, T> {
        using EB = B::EncodeBackendType;
        using DB = B::DecodeBackendType;

    public:
        using TargetType = T;
        using BackendType = B;

        ErrorCodec(const base::string& encode_message, const base::string& decode_message)
            : ErrorEncoder<EB, T>(encode_message), ErrorDecoder<DB, T>(decode_message) {}
        ErrorCodec(base::string_view encode_message, base::string_view decode_message)
            : ErrorEncoder<EB, T>(encode_message), ErrorDecoder<DB, T>(decode_message) {}
    };

    template <EncodeBackend EB>
    struct NullEncoder {
        using TargetType = base::nullptr_t;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, base::nullptr_t ptr = nullptr) {
            return backend.makeNull();
        }
    };

    template <DecodeBackend DB>
    struct NullDecoder {
        using TargetType = base::nullptr_t;
        using BackendType = DB;

        base::Either<base::nullptr_t, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            auto result = backend.getNull(data);
            if (!result.hasValue()) {
                return base::Left(nullptr);
            } else {
                return base::Right(base::move(result.value()));
            }
        }
    };

    template <CodecBackend B>
    struct NullCodec : NullEncoder<typename B::EncodeBackendType>, NullDecoder<typename B::DecodeBackendType> {
        using TargetType = base::nullptr_t;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct BooleanEncoder {
        using TargetType = bool;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, bool b) {
            return backend.makeBoolean(b);
        }
    };

    template <DecodeBackend DB>
    struct BooleanDecoder {
        using TargetType = bool;
        using BackendType = DB;

        base::Either<bool, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getBoolean(data);
        }
    };

    template <CodecBackend B>
    struct BooleanCodec : BooleanEncoder<typename B::EncodeBackendType>, BooleanDecoder<typename B::DecodeBackendType> {
        using TargetType = bool;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct I32Encoder {
        using TargetType = i32;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, i32 n) {
            return backend.makeI32(n);
        }
    };

    template <DecodeBackend DB>
    struct I32Decoder {
        using TargetType = i32;
        using BackendType = DB;

        base::Either<i32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getI32(data);
        }
    };

    template <CodecBackend B>
    struct I32Codec : I32Encoder<typename B::EncodeBackendType>, I32Decoder<typename B::DecodeBackendType> {
        using TargetType = i32;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct U32Encoder {
        using TargetType = u32;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, u32 n) {
            return backend.makeU32(n);
        }
    };

    template <DecodeBackend DB>
    struct U32Decoder {
        using TargetType = u32;
        using BackendType = DB;

        base::Either<u32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getU32(data);
        }
    };

    template <CodecBackend B>
    struct U32Codec : U32Encoder<typename B::EncodeBackendType>, U32Decoder<typename B::DecodeBackendType> {
        using TargetType = u32;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct F32Encoder {
        using TargetType = f32;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, f32 x) {
            return backend.makeF32(x);
        }
    };

    template <DecodeBackend DB>
    struct F32Decoder {
        using TargetType = f32;
        using BackendType = DB;

        base::Either<f32, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getF32(data);
        }
    };

    template <CodecBackend B>
    struct F32Codec : F32Encoder<typename B::EncodeBackendType>, F32Decoder<typename B::DecodeBackendType> {
        using TargetType = f32;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct F64Encoder {
        using TargetType = f64;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, f64 x) {
            return backend.makeF64(x);
        }
    };

    template <DecodeBackend DB>
    struct F64Decoder {
        using TargetType = f64;
        using BackendType = DB;

        base::Either<f64, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getF64(data);
        }
    };

    template <CodecBackend B>
    struct F64Codec : F64Encoder<typename B::EncodeBackendType>, F64Decoder<typename B::DecodeBackendType> {
        using TargetType = f64;
        using BackendType = B;
    };

    template <EncodeBackend EB>
    struct StringEncoder {
        using TargetType = base::string;
        using BackendType = EB;

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::string& s) {
            return backend.makeString(s);
        }
    };

    template <DecodeBackend DB>
    struct StringDecoder {
        using TargetType = base::string;
        using BackendType = DB;

        base::Either<base::string, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return backend.getString(data);
        }
    };

    template <CodecBackend B>
    struct StringCodec : StringEncoder<typename B::EncodeBackendType>, StringDecoder<typename B::DecodeBackendType> {
        using TargetType = base::string;
        using BackendType = B;
    };

    template <Encoder3 E>
    class ArrayEncoder {
        using T = typename E::TargetType;
        using EB = typename E::BackendType;
        using DataType = typename EB::DataType;
        using ErrorType = typename EB::ErrorType;

    public:
        using TargetType = base::vector<T>;
        using BackendType = EB;

        ArrayEncoder(const E& encoder) : _encoder(encoder) {}
        ArrayEncoder(E&& encoder) : _encoder(base::move(encoder)) {}

        base::Either<DataType, ErrorType> encode(EB& backend, const base::vector<T>& v) {
            base::vector<DataType> array;
            for (auto& item : v) {
                auto result = _encoder.encode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }
            return backend.makeArray(base::move(array));
        }

    private:
        E _encoder;
    };

    template <Encoder3 E>
    ArrayEncoder<E> makeArrayEncoder(const E& encoder) { return ArrayEncoder(encoder); }
    template <Encoder3 E>
    ArrayEncoder<E> makeArrayEncoder(E&& encoder) { return ArrayEncoder(base::move(encoder)); }

    template <Decoder3 D>
    class ArrayDecoder {
        using T = typename D::TargetType;
        using DB = typename D::BackendType;
        using DataType = typename DB::DataType;
        using ErrorType = typename DB::ErrorType;

    public:
        using TargetType = base::vector<T>;
        using BackendType = DB;

        ArrayDecoder(const D& decoder) : _decoder(decoder) {}
        ArrayDecoder(D&& decoder) : _decoder(base::move(decoder)) {}

        base::Either<base::vector<T>, ErrorType> decode(DB& backend, const DataType& data) {
            auto data_array = backend.getArray(data);
            if (data_array.isRight())
                return base::Right(base::move(data_array.right()));

            base::vector<T> array;
            for (auto& item : data_array.left()) {
                auto result = _decoder.decode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }

            return base::Left(base::move(array));
        }

    private:
        D _decoder;
    };

    template <Decoder3 D>
    ArrayDecoder<D> makeArrayDecoder(const D& decoder) { return ArrayDecoder(decoder); }
    template <Decoder3 D>
    ArrayDecoder<D> makeArrayDecoder(D&& decoder) { return ArrayDecoder(base::move(decoder)); }

    template <Codec C>
    class ArrayCodec : public ArrayEncoder<AsEncoder<C>>, public ArrayDecoder<AsDecoder<C>> {
        using E = AsEncoder<C>;
        using D = AsDecoder<C>;

    public:
        using TargetType = base::vector<typename C::TargetType>;
        using BackendType = C::BackendType;

        ArrayCodec(const C& codec) : ArrayEncoder<E>(E(codec)), ArrayDecoder<D>(D(codec)) {}
    };

    template <Codec C>
    ArrayCodec<C> makeArrayCodec(const C& codec) { return ArrayCodec(codec); }
    template <Codec C>
    ArrayCodec<C> makeArrayCodec(C&& codec) { return ArrayCodec(base::move(codec)); }

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
        typename E::BackendType;
    } && requires(E& encoder, E::BackendType& backend, RecordBuilderImpl<typename E::BackendType>& builder, const E::TargetType& object) {
        {
            encoder.encode(backend, builder, object)
        } -> base::SameAs<base::Optional<typename E::BackendType::ErrorType>>;
    };

    template <typename D>
    concept FieldDecoder = requires {
        typename D::TargetType;
        typename D::BackendType;
    } && requires(D& decoder, D::BackendType& backend, const D::BackendType::DataType& data) {
        {
            decoder.decode(backend, data)
        } -> base::SameAs<base::Either<typename D::TargetType, typename D::BackendType::ErrorType>>;
    };

    template <RecordBuilder RB, Encoder3 E>
        requires base::SameAs<typename RB::BackendType, typename E::BackendType>
    class NormalFieldEncoder {
        using T = E::TargetType;
        using EB = E::BackendType;

    public:
        using TargetType = T;
        using BackendType = EB;

        NormalFieldEncoder(base::string_view name, const E& encoder) : _name(name), _encoder(encoder) {}

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

    template <RecordBuilder RB, Encoder3 E>
        requires base::SameAs<typename RB::BackendType, typename E::BackendType>
    class OptionalFieldEncoder {
        using T = E::TargetType;
        using EB = E::BackendType;

    public:
        using TargetType = T;
        using BackendType = EB;

        OptionalFieldEncoder(base::string_view name, const E& encoder) : _name(name), _encoder(encoder) {}

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

    template <FieldEncoder E, typename S, typename T, typename F /* S -> T */>
    struct RecordEncoderBuilder {
        E encoder;
        F getter;
    };

    template <EncodeBackend EB, typename S, typename... Builders>
    class RecordEncoder {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        using TargetType = S;
        using BackendType = EB;

        RecordEncoder(const Builders&... builders) : _builders(builders...) {}
        RecordEncoder(Builders&&... builders) : _builders(base::move(builders)...) {}

        base::Either<DataType, ErrorType> encode(EB& backend, const S& object) {
            return std::apply(
                [&](const Builders&... builders) -> base::Either<DataType, ErrorType> {
                    RecordBuilderImpl<EB> record_builder;
                    base::Optional<ErrorType> error;

                    // magic fold expression that allows fast failing
                    bool success = ([&]() {
                        const auto& builder = builders;
                        auto result = builder.encoder->encode(backend, record_builder, builder.getter(object));
                        if (result.hasValue()) {
                            error = base::makeOptional<ErrorType>(base::move(result.value()));
                            // This will cause the execution to fail fast, preventing encoding further entries.
                            return false;
                        } else {
                            return true;
                        }
                    }() && ...);

                    if (success) {
                        return record_builder.build(backend);
                    } else {
                        return base::Right(base::move(error.value()));
                    }
                },
                _builders);
        }

    private:
        std::tuple<Builders...> _builders;
    };

}  // namespace spargel::codec
