/*
 * This implementation was inspired by https://github.com/Mojang/DataFixerUpper.
 * See LICENCE_Mojang_DataFixerUpper.
 */

#pragma once

#include <spargel/codec/codec.h>

// FIXME
#include <tuple>
#include <utility>

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

        template <typename E2>
        VectorEncoder(E2&& encoder) : _encoder(base::forward<E2>(encoder)) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::vector<T>& v) {
            return _vector::encodeVector(_encoder, backend, v);
        }

    private:
        E _encoder;
    };

    template <typename E>
    auto makeVectorEncoder(E&& encoder) { return VectorEncoder<base::remove_reference<E>>(base::forward<E>(encoder)); }

    template <Decoder4 D>
    class VectorDecoder {
        using T = typename D::TargetType;

    public:
        using TargetType = base::vector<T>;

        template <typename D2>
        VectorDecoder(D2&& decoder) : _decoder(base::forward<D2>(decoder)) {}

        template <DecodeBackend DB>
        base::Either<base::vector<T>, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _vector::decode(_decoder, backend, data);
        }

    private:
        D _decoder;
    };

    template <typename D>
    auto makeVectorDecoder(D&& decoder) { return VectorDecoder<base::remove_reference<D>>(base::forward<D>(decoder)); }

    template <Codec C>
    class VectorCodec {
        using T = typename C::TargetType;

    public:
        using TargetType = base::vector<T>;

        template <typename C2>
        VectorCodec(C2&& codec) : _codec(base::forward<C2>(codec)) {}

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

    template <typename C>
    auto makeVectorCodec(C&& codec) { return VectorCodec<base::remove_reference<C>>(base::forward<C>(codec)); }

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

    template <typename C>
    concept FieldCodec = FieldEncoder<C> && FieldDecoder<C>;

    namespace _field {

        template <Encoder4 E, EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encodeNormalField(E& encoder, base::string_view name, EB& backend, RB& builder, const typename E::TargetType& object) {
            auto result = encoder.encode(backend, object);
            if (result.isLeft()) {
                builder.add(name, base::move(result.left()));
                return base::nullopt;
            } else {
                return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
            }
        }

        template <Decoder4 D, DecodeBackend DB>
        base::Either<typename D::TargetType, typename DB::ErrorType> decodeNormalField(D& decoder, base::string_view name, DB& backend, const typename DB::DataType& data) {
            auto result = backend.getMember(data, name);
            if (result.isLeft()) {
                if (result.left().hasValue()) {
                    return decoder.decode(backend, base::move(result.left().value()));
                } else {
                    return base::Right(typename DB::ErrorType(base::string("cannot find member '") + name + '\''));
                }
            } else {
                return base::Right(base::move(result.right()));
            }
        }

        template <Decoder4 D, DecodeBackend DB>
        base::Either<typename D::TargetType, typename DB::ErrorType> decodeDefaultField(D& decoder, base::string_view name, DB& backend, const typename DB::DataType& data, const typename D::TargetType default_value) {
            auto result = backend.getMember(data, name);
            if (result.isLeft()) {
                if (result.left().hasValue()) {
                    return decoder.decode(backend, base::move(result.left().value()));
                } else {
                    return base::Left(default_value);
                }
            } else {
                return base::Right(base::move(result.right()));
            }
        }

        template <Encoder4 E, EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encodeOptionalField(E& encoder, base::string_view name, EB& backend, RB& builder, const base::Optional<typename E::TargetType>& object) {
            if (object.hasValue()) {
                auto result = encoder.encode(backend, object.value());
                if (result.isRight()) {
                    return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
                } else {
                    builder.add(name, base::move(result.left()));
                    return base::nullopt;
                }
            } else {
                return base::nullopt;
            }
        }

        template <Decoder4 D, DecodeBackend DB>
        base::Either<base::Optional<typename D::TargetType>, typename DB::ErrorType> decodeOptionalField(D& decoder, base::string_view name, DB& backend, const typename DB::DataType& data) {
            using T = D::TargetType;

            auto memberResult = backend.getMember(data, name);
            if (memberResult.isLeft()) {
                auto optional = base::move(memberResult.left());
                if (optional.hasValue()) {
                    auto result = decoder.decode(backend, optional.value());
                    if (result.isLeft()) {
                        return base::Left(base::makeOptional<T>(base::move(result.left())));
                    } else {
                        return base::Right(base::move(result.right()));
                    }
                } else {
                    return base::Left(base::Optional<T>());
                }
            } else {
                return base::Right(base::move(memberResult.right()));
            }
        }

    }  // namespace _field

    template <Encoder4 E>
    class NormalFieldEncoder {
        using T = E::TargetType;

    public:
        using TargetType = T;

        template <typename E2>
        NormalFieldEncoder(base::string_view name, E2&& encoder) : _name(name), _encoder(base::forward<E2>(encoder)) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const T& object) {
            return _field::encodeNormalField(_encoder, _name.view(), backend, builder, object);
        }

    private:
        base::string _name;
        E _encoder;
    };

    template <Decoder4 D>
    class NormalFieldDecoder {
        using T = D::TargetType;

    public:
        using TargetType = T;

        template <typename D2>
        NormalFieldDecoder(base::string_view name, D2&& decoder) : _name(name), _decoder(base::forward<D2>(decoder)) {}

        template <DecodeBackend DB>
        base::Either<typename D::TargetType, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeNormalField(_decoder, _name.view(), backend, data);
        }

    private:
        base::string _name;
        D _decoder;
    };

    template <Codec C>
    class NormalFieldCodec {
        using T = C::TargetType;

    public:
        using TargetType = T;

        template <typename C2>
        NormalFieldCodec(base::string_view name, C2&& codec) : _name(name), _codec(base::forward<C2>(codec)) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const T& object) {
            return _field::encodeNormalField(_codec, _name.view(), backend, builder, object);
        }

        template <DecodeBackend DB>
        base::Either<typename C::TargetType, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeNormalField(_codec, _name.view(), backend, data);
        }

    private:
        base::string _name;
        C _codec;
    };

    // There is no "DefaultFieldEncoder". It does not make sense.

    template <Decoder4 D>
    class DefaultFieldDecoder {
        using T = D::TargetType;

    public:
        using TargetType = T;

        template <typename D2>
        DefaultFieldDecoder(base::string_view name, D2&& decoder, const T& default_value) : _name(name), _decoder(base::forward<D2>(decoder)), _default_value(default_value) {}

        template <DecodeBackend DB>
        base::Either<typename D::TargetType, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeDefaultField(_decoder, _name.view(), backend, data, _default_value);
        }

    private:
        base::string _name;
        D _decoder;
        T _default_value;
    };

    template <Codec C>
    class DefaultFieldCodec {
        using T = C::TargetType;

    public:
        using TargetType = T;

        template <typename C2>
        DefaultFieldCodec(base::string_view name, C2&& codec, const T& default_value) : _name(name), _codec(base::forward<C2>(codec)), _default_value(default_value) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const T& object) {
            return _field::encodeNormalField(_codec, _name.view(), backend, builder, object);
        }

        template <DecodeBackend DB>
        base::Either<typename C::TargetType, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeDefaultField(_codec, _name.view(), backend, data, _default_value);
        }

    private:
        base::string _name;
        C _codec;
        T _default_value;
    };

    template <Encoder4 E>
    class OptionalFieldEncoder {
        using T = E::TargetType;

    public:
        using TargetType = base::Optional<T>;

        template <typename E2>
        OptionalFieldEncoder(base::string_view name, E2&& encoder) : _name(name), _encoder(base::forward<E2>(encoder)) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const base::Optional<T>& object) {
            return _field::encodeOptionalField(_encoder, _name.view(), backend, builder, object);
        }

    private:
        base::string _name;
        E _encoder;
    };

    template <Decoder4 D>
    class OptionalFieldDecoder {
        using T = D::TargetType;

    public:
        using TargetType = base::Optional<T>;

        template <typename D2>
        OptionalFieldDecoder(base::string_view name, D2&& decoder) : _name(name), _decoder(base::forward<D2>(decoder)) {}

        template <DecodeBackend DB>
        base::Either<typename base::Optional<T>, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeOptionalField(_decoder, _name.view(), backend, data);
        }

    private:
        base::string _name;
        D _decoder;
    };

    template <Codec C>
    class OptionalFieldCodec {
        using T = C::TargetType;

    public:
        using TargetType = base::Optional<T>;

        template <typename C2>
        OptionalFieldCodec(base::string_view name, C2&& codec) : _name(name), _codec(base::forward<C2>(codec)) {}

        template <EncodeBackend EB, RecordBuilder RB>
        base::Optional<typename EB::ErrorType> encode(EB& backend, RB& builder, const base::Optional<T>& object) {
            return _field::encodeOptionalField(_codec, _name.view(), backend, builder, object);
        }

        template <DecodeBackend DB>
        base::Either<typename base::Optional<T>, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _field::decodeOptionalField(_codec, _name.view(), backend, data);
        }

    private:
        base::string _name;
        C _codec;
    };

    template <FieldEncoder FE, typename F /* S -> T */>
    struct RecordEncoderBuilder {
        using TargetType = FE::TargetType;

        FE field_encoder;
        F getter;
    };

    // wrapper class for type match
    template <FieldDecoder FD>
    struct RecordDecoderBuilder {
        using TargetType = FD::TargetType;

        FD field_decoder;
    };

    template <FieldCodec FC, typename F /* S -> T */>
    struct RecordCodecBuilder {
        using TargetType = FC::TargetType;

        FC field_codec;
        F getter;
    };

    template <typename S, typename E, typename F /* S -> E::TargetType */>
    auto makeNormalEncodeField(base::string_view name, E&& encoder, F&& getter) {
        return RecordEncoderBuilder<NormalFieldEncoder<base::remove_reference<E>>, base::remove_reference<F>>{
            .field_encoder = NormalFieldEncoder<base::remove_reference<E>>(name, base::forward<E>(encoder)),
            .getter = getter};
    }

    template <typename D>
    auto makeNormalDecodeField(base::string_view name, D&& decoder) {
        return RecordDecoderBuilder<NormalFieldDecoder<base::remove_reference<D>>>{
            .field_decoder = NormalFieldDecoder<base::remove_reference<D>>(name, base::forward<D>(decoder))};
    }

    template <typename S, typename C, typename F /* S -> C::TargetType */>
    auto makeNormalField(base::string_view name, C&& codec, F&& getter) {
        return RecordCodecBuilder<NormalFieldCodec<base::remove_reference<C>>, base::remove_reference<F>>{
            .field_codec = NormalFieldCodec<base::remove_reference<C>>(name, base::forward<C>(codec)),
            .getter = getter};
    }

    template <typename D>
    auto makeDefaultDecodeField(base::string_view name, D&& decoder, const typename D::TargetType& default_value) {
        return RecordDecoderBuilder<DefaultFieldDecoder<base::remove_reference<D>>>{
            .field_decoder = DefaultFieldDecoder<base::remove_reference<D>>(name, base::forward<D>(decoder), default_value)};
    }

    template <typename S, typename C, typename F /* S -> C::TargetType */>
    auto makeDefaultField(base::string_view name, C&& codec, const typename C::TargetType& default_value, F&& getter) {
        return RecordCodecBuilder<DefaultFieldCodec<base::remove_reference<C>>, base::remove_reference<F>>{
            .field_codec = DefaultFieldCodec<base::remove_reference<C>>(name, base::forward<C>(codec), default_value),
            .getter = getter};
    }

    template <typename S, typename E, typename F /* S -> E::TargetType */>
    auto makeOptionalEncodeField(base::string_view name, E&& encoder, F&& getter) {
        return RecordEncoderBuilder<OptionalFieldEncoder<base::remove_reference<E>>, base::remove_reference<F>>{
            .field_encoder = OptionalFieldEncoder<base::remove_reference<E>>(name, base::forward<E>(encoder)),
            .getter = getter};
    }

    template <typename D>
    auto makeOptionalDecodeField(base::string_view name, D&& decoder) {
        return RecordDecoderBuilder<OptionalFieldDecoder<base::remove_reference<D>>>{
            .field_decoder = OptionalFieldDecoder<base::remove_reference<D>>(name, base::forward<D>(decoder))};
    }

    template <typename S, typename C, typename F /* S -> C::TargetType */>
    auto makeOptionalField(base::string_view name, C&& codec, F&& getter) {
        return RecordCodecBuilder<OptionalFieldCodec<base::remove_reference<C>>, base::remove_reference<F>>{
            .field_codec = OptionalFieldCodec<base::remove_reference<C>>(name, base::forward<C>(codec)),
            .getter = getter};
    }

    namespace _record {

        template <typename T /* RecordEncoderBuilder || RecordCodecBuilder */>
        struct EncoderFrom;

        template <FieldEncoder FE, typename F>
        struct EncoderFrom<RecordEncoderBuilder<FE, F>> {
            EncoderFrom(RecordEncoderBuilder<FE, F>& builder) : encoder(builder.field_encoder) {}

            FE& encoder;
        };

        template <FieldCodec FC, typename F>
        struct EncoderFrom<RecordCodecBuilder<FC, F>> {
            EncoderFrom(RecordCodecBuilder<FC, F>& builder) : encoder(builder.field_codec) {}

            FC& encoder;
        };

        template <typename T /* RecordDecoderBuilder || RecordCodecBuilder */>
        struct DecoderFrom;

        template <FieldDecoder FD>
        struct DecoderFrom<RecordDecoderBuilder<FD>> {
            DecoderFrom(RecordDecoderBuilder<FD>& builder) : decoder(builder.field_decoder) {}

            FD& decoder;
        };

        template <FieldCodec FC, typename F>
        struct DecoderFrom<RecordCodecBuilder<FC, F>> {
            DecoderFrom(RecordCodecBuilder<FC, F>& builder) : decoder(builder.field_codec) {}

            FC& decoder;
        };

        template <EncodeBackend EB, typename S, typename... Builders /* RecordEncoderBuilder || RecordCodecBuilder */>
        base::Either<typename EB::DataType, typename EB::ErrorType> encodeRecord(EB& backend, const S& object, std::tuple<Builders...>& builders) {
            using DataType = EB::DataType;
            using ErrorType = EB::ErrorType;

            return std::apply(
                [&](Builders&... builders) -> base::Either<DataType, ErrorType> {
                    RecordBuilderImpl<EB> record_builder;
                    base::Optional<ErrorType> error;

                    // magic fold expression that allows fast failing
                    bool success = ([&]() {
                        auto& builder = builders;
                        auto result = EncoderFrom<base::remove_reference<decltype(builder)>>(builder).encoder.encode(backend, record_builder, builder.getter(object));
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
                builders);
        }

        template <DecodeBackend DB, typename S, typename F, typename... Builders, typename I, I... indices>
        base::Either<S, typename DB::ErrorType> decodeRecordImpl(DB& backend, const typename DB::DataType& data, F& func, std::tuple<Builders...>& builders, std::integer_sequence<I, indices...>) {
            using ErrorType = DB::ErrorType;

            base::Optional<ErrorType> error;

            // This hold decoded member values.
            std::tuple<base::Optional<typename Builders::TargetType>...> values;

            bool success = ([&]() {
                const auto i = indices;
                auto& builder = std::get<i>(builders);
                auto result = DecoderFrom<base::remove_reference<decltype(builder)>>(builder).decoder.decode(backend, data);
                using Type = base::Get<base::TypeList<typename Builders::TargetType...>, i>;
                if (result.isLeft()) {
                    std::get<i>(values) = base::makeOptional<Type>(base::move(result.left()));
                    return true;
                } else {
                    error = base::makeOptional<ErrorType>(base::move(result.right()));
                    return false;
                }
            }() && ...);

            if (success) {
                return base::Left(std::apply(
                    [&](const base::Optional<typename Builders::TargetType>&... args) {
                        return func(args.value()...);
                    },
                    base::move(values)));
            } else {
                return base::Right(base::move(error.value()));
            }
        }

        template <DecodeBackend DB, typename S, typename F, typename... Builders /* RecordDecoderBuilder || RecordCodecBuilder */>
        base::Either<S, typename DB::ErrorType> decodeRecord(DB& backend, const typename DB::DataType& data, F& func, std::tuple<Builders...>& decoders) {
            return decodeRecordImpl<DB, S, F>(backend, data, func, decoders, std::index_sequence_for<Builders...>{});
        }

    }  // namespace _record

    template <typename S, typename... Builders /* RecordEncoderBuilder || RecordCodecBuilder */>
    class RecordEncoder {
    public:
        using TargetType = S;

        template <typename... Builders2>
        RecordEncoder(Builders2&&... builders) : _builders(base::forward<Builders2>(builders)...) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const S& object) {
            return _record::encodeRecord<EB, S, Builders...>(backend, object, _builders);
        }

    private:
        std::tuple<Builders...> _builders;
    };

    template <typename S, typename... Builders>
    auto makeRecordEncoder(Builders&&... builders) {
        return RecordEncoder<S, base::remove_reference<Builders>...>(base::forward<Builders>(builders)...);
    }

    template <typename S, typename F /* (Ts...) -> S */, typename... Builders /* RecordDecoderBuilder || RecordCodecBuilder */>
    class RecordDecoder {
    public:
        using TargetType = S;

        template <typename... Builders2>
        RecordDecoder(F&& func, Builders2&&... decoders) : _func(base::forward<F>(func)), _decoders(base::forward<Builders2>(decoders)...) {}

        template <DecodeBackend DB>
        base::Either<S, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _record::decodeRecord<DB, S, F, Builders...>(backend, data, _func, _decoders);
        }

    private:
        F _func;
        std::tuple<Builders...> _decoders;
    };

    template <typename S, typename F, typename... Builders>
    auto makeRecordDecoder(F&& func, Builders&&... builders) {
        return RecordDecoder<S, base::remove_reference<F>, base::remove_reference<Builders>...>(base::forward<F>(func), base::forward<Builders>(builders)...);
    }

    template <typename S, typename F /* (Ts...) -> S */, typename... Builders /* RecordCodecBuilder */>
    class RecordCodec {
    public:
        using TargetType = S;

        template <typename... Builders2>
        RecordCodec(F&& func, Builders2&&... builders) : _func(base::forward<F>(func)), _builders(base::forward<Builders2>(builders)...) {}

        template <EncodeBackend EB>
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const S& object) {
            return _record::encodeRecord<EB, S, Builders...>(backend, object, _builders);
        }

        template <DecodeBackend DB>
        base::Either<S, typename DB::ErrorType> decode(DB& backend, const typename DB::DataType& data) {
            return _record::decodeRecord<DB, S, F, Builders...>(backend, data, _func, _builders);
        }

    private:
        F _func;
        std::tuple<Builders...> _builders;
    };

    template <typename S, typename F, typename... Builders>
    auto makeRecordCodec(F&& func, Builders&&... builders) {
        return RecordCodec<S, base::remove_reference<F>, base::remove_reference<Builders>...>(base::forward<F>(func), base::forward<Builders>(builders)...);
    }

}  // namespace spargel::codec
