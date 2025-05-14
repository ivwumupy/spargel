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

    // encoder prototype
    template <typename E>
    concept Encoder3 = requires {
        typename E::TargetType;
        typename E::BackendType;
    } && requires(E& encoder, E::BackendType& backend, const E::TargetType& object) {
        {
            encoder.encode(backend, object)
        } -> base::ConvertibleTo<base::Either<typename E::BackendType::DataType, typename E::BackendType::ErrorType>>;
    };

    // decoder prototype
    template <typename D>
    concept Decoder3 = requires {
        typename D::TargetType;
        typename D::BackendType;
    } && requires(D& decoder, D::BackendType& backend, const D::BackendType::DataType& data) {
        {
            decoder.decode(backend, data)
        } -> base::ConvertibleTo<base::Either<typename D::TargetType, typename D::BackendType::ErrorType>>;
    };

    template <typename C>
    concept Codec = requires {
        typename C::TargetType;
        typename C::BackendType;
    } && requires(C& codec, C::BackendType::EncodeBackendType& backend, const C::TargetType& object) {
        {
            codec.encode(backend, object)
        } -> base::ConvertibleTo<base::Either<typename C::BackendType::EncodeBackendType::DataType, typename C::BackendType::EncodeBackendType::ErrorType>>;
    } && requires(C& codec, C::BackendType::DecodeBackendType& backend, const C::BackendType::DecodeBackendType::DataType& data) {
        {
            codec.decode(backend, data)
        } -> base::ConvertibleTo<base::Either<typename C::TargetType, typename C::BackendType::DecodeBackendType::ErrorType>>;
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

        ArrayCodec(const C& codec) : ArrayEncoder<AsEncoder<C>>(codec), ArrayDecoder<AsDecoder<C>>(codec) {}
    };

    template <Codec C>
    ArrayCodec<C> makeArrayCodec(const C& codec) { return ArrayCodec(codec); }
    template <Codec C>
    ArrayCodec<C> makeArrayCodec(C&& codec) { return ArrayCodec(base::move(codec)); }

}  // namespace spargel::codec
