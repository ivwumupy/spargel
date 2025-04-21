/*
 * This implementation was inspired by https://github.com/Mojang/DataFixerUpper.
 * See LICENCE_Mojang_DataFixerUpper.
 */

#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/codec/codec.h>

// FIXME
#include <tuple>
#include <utility>

namespace spargel::codec {

    template <EncodeBackend EB, DecodeBackend DB>
    struct CodecBackend {
        EB encodeBackend;
        DB decodeBackend;
    };

    template <EncodeBackend EB>
    class RecordBuilder {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        virtual ~RecordBuilder() = default;

        virtual void add(const base::string& name, const DataType& value) = 0;
        virtual void add(const base::string& name, DataType&& value) = 0;

        virtual base::Either<DataType, ErrorType> build(EB& backend) const = 0;
    };

    template <EncodeBackend EB>
    class RecordBuilderImpl : public RecordBuilder<EB> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        RecordBuilderImpl() : _map(base::default_allocator()) {}

        void add(const base::string& name, const DataType& value) override {
            _map.set(name, value);
        }
        void add(const base::string& name, DataType&& value) override {
            _map.set(name, base::move(value));
        }

        base::Either<DataType, ErrorType> build(EB& backend) const override {
            return backend.makeMap(_map);
        }

    private:
        base::HashMap<base::string, DataType> _map;
    };

    template <EncodeBackend EB, typename T>
    class ArrayEncoder;
    template <DecodeBackend DB, typename T>
    class ArrayDecoder;
    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class ArrayCodec;

    template <EncodeBackend EB, typename T>
    class NormalFieldEncoder;
    template <DecodeBackend DB, typename T>
    class NormalFieldDecoder;
    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class NormalFieldCodec;

    template <EncodeBackend EB, typename T>
    class OptionalFieldEncoder;
    template <DecodeBackend DB, typename T>
    class OptionalFieldDecoder;
    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class OptionalFieldCodec;

    template <EncodeBackend EB, typename S, typename T, typename F>
    class RecordEncoderBuilder;

    template <EncodeBackend EB, typename T>
    class Encoder2 {
    public:
        using type = T;

        virtual ~Encoder2() = default;

        virtual base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) const = 0;

        virtual base::unique_ptr<Encoder2> clone() const = 0;

        ArrayEncoder<EB, T> arrayOf() { return ArrayEncoder<EB, T>(clone()); }

        NormalFieldEncoder<EB, T> fieldOf(base::string_view name) const {
            return NormalFieldEncoder<EB, T>(name, clone());
        }

        OptionalFieldEncoder<EB, T> optionalFieldOf(base::string_view name) const {
            return OptionalFieldEncoder<EB, T>(name, clone());
        }
    };

    template <DecodeBackend DB, typename T>
    class Decoder2 {
    public:
        using type = T;

        virtual ~Decoder2() = default;

        virtual base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const = 0;

        virtual base::unique_ptr<Decoder2> clone() const = 0;

        ArrayDecoder<DB, T> arrayOf() { return ArrayDecoder<DB, T>(clone()); }

        NormalFieldDecoder<DB, T> fieldOf(base::string_view name) const {
            return NormalFieldDecoder<DB, T>(name, clone());
        }

        OptionalFieldDecoder<DB, T> optionalFieldOf(base::string_view name) const {
            return OptionalFieldDecoder<DB, T>(name, clone());
        }
    };

    template <EncodeBackend EB, DecodeBackend DB, typename T>
    struct Codec {
    public:
        using type = T;

        Codec(base::unique_ptr<Encoder2<EB, T>>&& encoder, base::unique_ptr<Decoder2<DB, T>>&& decoder)
            : encoder(base::move(encoder)), decoder(base::move(decoder)) {}

        virtual ~Codec() = default;

        virtual base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) const {
            return encoder->encode(backend, object);
        }

        virtual base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const {
            return decoder->decode(backend, data);
        }

        virtual base::unique_ptr<Codec> clone() const {
            return base::make_unique<Codec>(encoder->clone(), decoder->clone());
        }

        ArrayCodec<EB, DB, T> arrayOf() {
            return ArrayCodec<EB, DB, T>(encoder->clone(), decoder->clone());
        }

        NormalFieldCodec<EB, DB, T> fieldOf(base::string_view name) const {
            return NormalFieldCodec<EB, DB, T>(name, clone());
        }

        OptionalFieldCodec<EB, DB, T> optionalFieldOf(base::string_view name) const {
            return OptionalFieldCodec<EB, DB, T>(name, clone());
        }

    protected:
        base::unique_ptr<Encoder2<EB, T>> encoder;
        base::unique_ptr<Decoder2<DB, T>> decoder;
    };

    /*
     * Helper class for combining an encoder and an decoder that are trivially constructable.
     */
    template <EncodeBackend EB, DecodeBackend DB, typename T, typename E, typename D>
    // requires IsBaseOf<Encoder<EB, T>, E> && requires IsBaseOf<Decoder<DB, T>, D>
    struct SimpleCodec : Codec<EB, DB, T> {
    public:
        SimpleCodec() : Codec<EB, DB, T>(base::make_unique<E>(), base::make_unique<D>()) {}
    };

    template <EncodeBackend EB, typename T>
    class ErrorEncoder : public Encoder2<EB, T> {
    public:
        ErrorEncoder(const base::string& message) : _message(message) {}
        ErrorEncoder(base::string_view message) : _message(message) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) const override {
            return base::Right(typename EB::ErrorType(_message));
        }

        virtual base::unique_ptr<Encoder2<EB, T>> clone() const override {
            return base::make_unique<ErrorEncoder>(_message);
        }

    private:
        base::string _message;
    };
    template <DecodeBackend DB, typename T>
    class ErrorDecoder : public Decoder2<DB, T> {
    public:
        ErrorDecoder(const base::string& message) : _message(message) {}
        ErrorDecoder(base::string_view message) : _message(message) {}

        base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return base::Right(typename DB::ErrorType(_message));
        }

        virtual base::unique_ptr<Decoder2<DB, T>> clone() const override {
            return base::make_unique<ErrorDecoder>(_message);
        }

    private:
        base::string _message;
    };
    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class ErrorCodec : public Codec<EB, DB, T> {
    public:
        ErrorCodec(base::string_view encode_message, base::string_view decode_message)
            : Codec<EB, DB, T>(
                  base::make_unique<ErrorEncoder<EB, T>>(encode_message),
                  base::make_unique<ErrorDecoder<DB, T>>(decode_message)) {}
        ErrorCodec(const base::string& encode_message, const base::string& decode_message)
            : ErrorCodec(encode_message.view(), decode_message.view()) {}
    };

    // base::nullptr_t
    template <EncodeBackend EB>
    class NullEncoder : public Encoder2<EB, base::nullptr_t> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::nullptr_t& ptr) const override {
            return backend.makeNull();
        }

        base::unique_ptr<Encoder2<EB, base::nullptr_t>> clone() const override {
            return base::make_unique<NullEncoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class NullDecoder : public Decoder2<DB, base::nullptr_t> {
    public:
        base::Either<base::nullptr_t, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            auto result = backend.getNull(data);
            if (!result.hasValue()) {
                return base::Left(nullptr);
            } else {
                return base::Right(base::move(result.value()));
            }
        }

        base::unique_ptr<Decoder2<DB, base::nullptr_t>> clone() const override {
            return base::make_unique<NullDecoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using NullCodec = SimpleCodec<EB, DB, base::nullptr_t, NullEncoder<EB>, NullDecoder<DB>>;

    template <EncodeBackend EB>
    class BooleanEncoder : public Encoder2<EB, bool> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const bool& b) const override {
            return backend.makeBoolean(b);
        }

        base::unique_ptr<Encoder2<EB, bool>> clone() const override {
            return base::make_unique<BooleanEncoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class BooleanDecoder : public Decoder2<DB, bool> {
    public:
        base::Either<bool, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return backend.getBoolean(data);
        }

        base::unique_ptr<Decoder2<DB, bool>> clone() const override {
            return base::make_unique<BooleanDecoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using BooleanCodec = SimpleCodec<EB, DB, bool, BooleanEncoder<EB>, BooleanDecoder<DB>>;

    template <EncodeBackend EB>
    class U32Encoder : public Encoder2<EB, u32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const u32& n) const override {
            return backend.makeU32(n);
        }

        base::unique_ptr<Encoder2<EB, u32>> clone() const override {
            return base::make_unique<U32Encoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class U32Decoder : public Decoder2<DB, u32> {
    public:
        base::Either<u32, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return backend.getU32(data);
        }

        base::unique_ptr<Decoder2<DB, u32>> clone() const override {
            return base::make_unique<U32Decoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using U32Codec = SimpleCodec<EB, DB, u32, U32Encoder<EB>, U32Decoder<DB>>;

    template <EncodeBackend EB>
    class I32Encoder : public Encoder2<EB, i32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const i32& n) const override {
            return backend.makeI32(n);
        }

        base::unique_ptr<Encoder2<EB, i32>> clone() const override {
            return base::make_unique<I32Encoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class I32Decoder : public Decoder2<DB, i32> {
    public:
        base::Either<i32, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return backend.getI32(data);
        }

        base::unique_ptr<Decoder2<DB, i32>> clone() const override {
            return base::make_unique<I32Decoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using I32Codec = SimpleCodec<EB, DB, i32, I32Encoder<EB>, I32Decoder<DB>>;

    template <EncodeBackend EB>
    class F32Encoder : public Encoder2<EB, f32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const f32& n) const override {
            return backend.makeF32(n);
        }

        base::unique_ptr<Encoder2<EB, f32>> clone() const override {
            return base::make_unique<F32Encoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class F32Decoder : public Decoder2<DB, f32> {
    public:
        base::Either<f32, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return backend.getF32(data);
        }

        base::unique_ptr<Decoder2<DB, f32>> clone() const override {
            return base::make_unique<F32Decoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using F32Codec = SimpleCodec<EB, DB, f32, F32Encoder<EB>, F32Decoder<DB>>;

    template <EncodeBackend EB>
    class StringEncoder : public Encoder2<EB, base::string> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::string& s) const override {
            return backend.makeString(s);
        }

        base::unique_ptr<Encoder2<EB, base::string>> clone() const override {
            return base::make_unique<StringEncoder<EB>>();
        }
    };
    template <DecodeBackend DB>
    class StringDecoder : public Decoder2<DB, base::string> {
    public:
        base::Either<base::string, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            return backend.getString(data);
        }

        base::unique_ptr<Decoder2<DB, base::string>> clone() const override {
            return base::make_unique<StringDecoder<DB>>();
        }
    };
    template <EncodeBackend EB, DecodeBackend DB>
    using StringCodec = SimpleCodec<EB, DB, base::string, StringEncoder<EB>, StringDecoder<DB>>;

    template <EncodeBackend EB, typename T>
    class ArrayEncoder : public Encoder2<EB, base::vector<T>> {
    public:
        ArrayEncoder(base::unique_ptr<Encoder2<EB, T>>&& encoder) : _encoder(base::move(encoder)) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const base::vector<T>& v) const override {
            base::vector<typename EB::DataType> array;
            for (auto& item : v) {
                auto result = _encoder->encode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }
            return backend.makeArray(base::move(array));
        }

        base::unique_ptr<Encoder2<EB, base::vector<T>>> clone() const override {
            return base::make_unique<ArrayEncoder>(_encoder->clone());
        }

        base::unique_ptr<Encoder2<EB, T>> encoder() { return _encoder->clone(); }

    private:
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };
    template <DecodeBackend DB, typename T>
    class ArrayDecoder : public Decoder2<DB, base::vector<T>> {
    public:
        ArrayDecoder(base::unique_ptr<Decoder2<DB, T>>&& decoder) : _decoder(base::move(decoder)) {}

        base::Either<base::vector<T>, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            auto result_array = backend.getArray(data);
            if (result_array.isRight())
                return base::Right(base::move(result_array.right()));

            base::vector<T> array;
            for (auto& item : result_array.left()) {
                auto result = _decoder->decode(backend, item);
                if (result.isRight())
                    return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }

            return base::Left(base::move(array));
        }

        base::unique_ptr<Decoder2<DB, base::vector<T>>> clone() const override {
            return base::make_unique<ArrayDecoder>(_decoder->clone());
        }

        base::unique_ptr<Decoder2<DB, T>> decoder() { return _decoder->clone(); }

    private:
        base::unique_ptr<Decoder2<DB, T>> _decoder;
    };
    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class ArrayCodec : public Codec<EB, DB, base::vector<T>> {
    public:
        ArrayCodec(base::unique_ptr<Encoder2<EB, T>>&& encoder, base::unique_ptr<Decoder2<DB, T>>&& decoder)
            : Codec<EB, DB, base::vector<T>>(
                  base::make_unique<ArrayEncoder<EB, T>>(base::move(encoder)),
                  base::make_unique<ArrayDecoder<DB, T>>(base::move(decoder))) {}
    };

    template <EncodeBackend EB, typename T>
    class FieldEncoder {
    public:
        using type = T;

        virtual ~FieldEncoder() = default;

        virtual base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const = 0;

        virtual base::unique_ptr<FieldEncoder> clone() const = 0;

        template <typename S, typename F /* S -> T */>
        RecordEncoderBuilder<EB, S, T, F> forGetter(F getter) const {
            return RecordEncoderBuilder<EB, S, T, F>(clone(), getter);
        }
    };

    template <DecodeBackend DB, typename T>
    class FieldDecoder {
    public:
        using type = T;

        virtual ~FieldDecoder() = default;

        virtual base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const = 0;

        virtual base::unique_ptr<FieldDecoder> clone() const = 0;
    };

    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class FieldCodec {
    public:
        using type = T;

        FieldCodec(base::unique_ptr<FieldEncoder<EB, T>>&& encoder, base::unique_ptr<FieldDecoder<DB, T>> decoder)
            : encoder(base::move(encoder)), decoder(base::move(decoder)) {}

        virtual ~FieldCodec() = default;

        virtual base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const {
            return encoder->encode(backend, builder, object);
        }

        virtual base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const {
            return decoder->decode(backend, data);
        }

        virtual base::unique_ptr<FieldCodec> clone() const {
            return base::make_unique<FieldCodec>(encoder->clone(), decoder->clone());
        }

    protected:
        base::unique_ptr<FieldEncoder<EB, T>> encoder;
        base::unique_ptr<FieldDecoder<DB, T>> decoder;
    };

    template <EncodeBackend EB, typename T>
    class NormalFieldEncoder : public FieldEncoder<EB, T> {
    public:
        NormalFieldEncoder(base::string_view name, base::unique_ptr<Encoder2<EB, T>> encoder)
            : _name(name), _encoder(base::move(encoder)) {}

        base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const override {
            auto result = _encoder->encode(backend, object);
            if (result.isLeft()) {
                builder.add(_name, base::move(result.left()));
                return base::nullopt;
            } else {
                return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
            }
        }

        base::unique_ptr<FieldEncoder<EB, T>> clone() const override {
            return base::make_unique<NormalFieldEncoder<EB, T>>(_name.view(), _encoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <DecodeBackend DB, typename T>
    class NormalFieldDecoder : public FieldDecoder<DB, T> {
    public:
        using base_type = FieldDecoder<DB, T>;

        NormalFieldDecoder(base::string_view name, base::unique_ptr<Decoder2<DB, T>> decoder)
            : _name(name), _decoder(base::move(decoder)) {}

        base::Either<T, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            auto memberResult = backend.getMember(data, _name.view());
            if (memberResult.isLeft()) {
                auto optional = base::move(memberResult.left());
                if (optional.hasValue()) {
                    return _decoder->decode(backend, optional.value());
                } else {
                    return base::Right(typename DB::ErrorType(base::string("cannot find member '") + _name + '\''));
                }
            } else {
                return base::Right(base::move(memberResult.right()));
            }
        }

        base::unique_ptr<FieldDecoder<DB, T>> clone() const override {
            return base::make_unique<NormalFieldDecoder<DB, T>>(_name.view(), _decoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Decoder2<DB, T>> _decoder;
    };

    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class NormalFieldCodec : public FieldCodec<EB, DB, T> {
    public:
        NormalFieldCodec(base::string_view name, base::unique_ptr<Encoder2<EB, T>>&& encoder, base::unique_ptr<Decoder2<DB, T>>&& decoder)
            : FieldCodec<EB, DB, base::vector<T>>(
                  base::make_unique<NormalFieldEncoder<EB, T>>(name, base::move(encoder)),
                  base::make_unique<NormalFieldDecoder<DB, T>>(name, base::move(decoder))) {}
    };

    template <EncodeBackend EB, typename T>
    class OptionalFieldEncoder : public FieldEncoder<EB, base::Optional<T>> {
    public:
        OptionalFieldEncoder(const base::string& name, base::unique_ptr<Encoder2<EB, T>> encoder)
            : _name(name), _encoder(base::move(encoder)) {}

        base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const base::Optional<T>& object) const override {
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

        base::unique_ptr<FieldEncoder<EB, base::Optional<T>>> clone() const override {
            return base::make_unique<OptionalFieldEncoder<EB, T>>(_name, _encoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <DecodeBackend DB, typename T>
    class OptionalFieldDecoder : public FieldDecoder<DB, base::Optional<T>> {
    public:
        using base_type = FieldDecoder<DB, base::Optional<T>>;

        OptionalFieldDecoder(base::string_view name, base::unique_ptr<Decoder2<DB, T>> decoder)
            : _name(name), _decoder(base::move(decoder)) {}

        base::Either<base::Optional<T>, typename DB::ErrorType> decode(DB& backend, const DB::DataType& data) const override {
            auto memberResult = backend.getMember(data, _name.view());
            if (memberResult.isLeft()) {
                auto optional = base::move(memberResult.left());
                if (optional.hasValue()) {
                    auto result = _decoder->decode(backend, optional.value());
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

        base::unique_ptr<FieldDecoder<DB, base::Optional<T>>> clone() const override {
            return base::make_unique<OptionalFieldDecoder<DB, T>>(_name.view(), _decoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Decoder2<DB, T>> _decoder;
    };

    template <EncodeBackend EB, DecodeBackend DB, typename T>
    class OptionalFieldCodec : public FieldCodec<EB, DB, T> {
    public:
        OptionalFieldCodec(base::string_view name, base::unique_ptr<Encoder2<EB, T>>&& encoder, base::unique_ptr<Decoder2<DB, T>>&& decoder)
            : FieldCodec<EB, DB, base::vector<T>>(
                  base::make_unique<OptionalFieldEncoder<EB, T>>(name, base::move(encoder)),
                  base::make_unique<OptionalFieldDecoder<DB, T>>(name, base::move(decoder))) {}
    };

    template <EncodeBackend EB, typename S, typename T, typename F /* S -> T */>
    class RecordEncoderBuilder {
    public:
        RecordEncoderBuilder(base::unique_ptr<FieldEncoder<EB, T>>&& field_encoder, F getter)
            : field_encoder(base::move(field_encoder)), getter(getter) {}

        RecordEncoderBuilder(const RecordEncoderBuilder& that) : field_encoder(that.field_encoder->clone()), getter(that.getter) {}
        RecordEncoderBuilder(RecordEncoderBuilder&& that) = default;

    public:
        base::unique_ptr<FieldEncoder<EB, T>> field_encoder;
        F getter;
    };

    template <EncodeBackend EB, typename S, typename... Builders>
    class RecordEncoder : public Encoder2<EB, S> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        RecordEncoder(const Builders&... builders) : _builders(builders...) {}
        RecordEncoder(Builders&&... builders) : _builders(base::move(builders)...) {}

        base::Either<DataType, ErrorType> encode(EB& backend, const S& object) const override {
            return std::apply(
                [&](const Builders&... builders) -> base::Either<DataType, ErrorType> {
                    RecordBuilderImpl<EB> record_builder;
                    base::Optional<ErrorType> error;

                    // magic fold expression that allows fast failing
                    bool success = ([&]() {
                        const auto& builder = builders;
                        auto result = builder.field_encoder->encode(backend, record_builder, builder.getter(object));
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

        base::unique_ptr<Encoder2<EB, S>> clone() const override {
            return std::apply([](const Builders&... builders) { return base::make_unique<RecordEncoder>(builders...); }, _builders);
        }

    private:
        std::tuple<Builders...> _builders;
    };

    template <EncodeBackend EB, typename S, typename... Builders>
    static RecordEncoder<EB, S, Builders...> makeRecordEncoder(const Builders&... builders) {
        return RecordEncoder<EB, S, Builders...>(builders...);
    }
    template <EncodeBackend EB, typename S, typename... Builders>
    static RecordEncoder<EB, S, Builders...> makeRecordEncoder(Builders&&... builders) {
        return RecordEncoder<EB, S, Builders...>(base::move(builders)...);
    }

    template <DecodeBackend DB, typename S, typename F /* (Ts...) -> S */, typename... Decoders>
    class RecordDecoder : public Decoder2<DB, S> {
        using DataType = DB::DataType;
        using ErrorType = DB::ErrorType;

    public:
        RecordDecoder(F&& func, base::unique_ptr<typename Decoders::base_type>&&... decoders) : _func(base::move(func)), _decoders(base::move(decoders)...) {}

        base::unique_ptr<Decoder2<DB, S>> clone() const override {
            return std::apply(
                [&](const base::unique_ptr<typename Decoders::base_type>&... decoders) {
                    return base::make_unique<RecordDecoder>(F(_func), decoders->clone()...);
                },
                _decoders);
        }

        base::Either<S, ErrorType> decode(DB& backend, const DataType& data) const override {
            return decodeImpl(backend, data, std::index_sequence_for<Decoders...>{});
        }

    private:
        F _func;
        std::tuple<base::unique_ptr<typename Decoders::base_type>...> _decoders;

        template <typename T, T... indices>
        base::Either<S, ErrorType> decodeImpl(DB& backend, const DataType& data, std::integer_sequence<T, indices...>) const {
            base::Optional<ErrorType> error;
            std::tuple<base::Optional<typename Decoders::type>...> values;

            bool success = ([&]() {
                const auto i = indices;
                auto& decoder = std::get<i>(_decoders);
                auto result = decoder->decode(backend, data);
                using Type = base::Get<base::TypeList<typename Decoders::type...>, i>;
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
                    [&](const base::Optional<typename Decoders::type>&... args) {
                        return _func(args.value()...);
                    },
                    base::move(values)));
            } else {
                return base::Right(base::move(error.value()));
            }
        }
    };

    template <DecodeBackend DB, typename S, typename F, typename... Decoders>
    static RecordDecoder<DB, S, F, Decoders...> makeRecordDecoder(F&& func, const Decoders&... decoders) {
        return RecordDecoder<DB, S, F, Decoders...>(base::move(func), decoders.clone()...);
    }

}  // namespace spargel::codec
