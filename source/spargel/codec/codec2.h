/*
 * This implementation was inspired by https://github.com/Mojang/DataFixerUpper.
 * See LICENCE_Mojang_DataFixerUpper.
 */

#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/codec/codec.h>

// FIXME
#include <tuple>

namespace spargel::codec {

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
    class Encoder2;
    template <EncodeBackend EB, typename T>
    class ArrayEncoder;
    template <EncodeBackend EB, typename S, typename T, typename F>
    class RecordEncoderBuilder;
    template <EncodeBackend EB, typename T>
    class NormalFieldEncoder;
    template <EncodeBackend EB, typename T>
    class OptionalFieldEncoder;

    template <EncodeBackend EB, typename T>
    class Encoder2 {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        virtual ~Encoder2() = default;

        virtual base::Either<DataType, ErrorType> encode(EB& backend, const T& object) const = 0;

        virtual base::unique_ptr<Encoder2> clone() const = 0;

        NormalFieldEncoder<EB, T> fieldOf(base::string_view name) const {
            return NormalFieldEncoder<EB, T>(name, clone());
        }

        OptionalFieldEncoder<EB, T> optionalFieldOf(base::string_view name) const {
            return OptionalFieldEncoder<EB, T>(name, clone());
        }

        ArrayEncoder<EB, T> arrayOf() { return ArrayEncoder<EB, T>(clone()); }
    };

    template <EncodeBackend EB, typename T>
    class ErrorEncoder : public Encoder2<EB, T> {
    public:
        ErrorEncoder(const base::string& message) : _message(message) {}

        ErrorEncoder(const char* message) : _message(message) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend, const T& object) const override {
            return base::Right(typename EB::ErrorType(_message));
        }

        virtual base::unique_ptr<Encoder2<EB, T>> clone() const override {
            return base::make_unique<ErrorEncoder>(_message);
        }

    private:
        base::string _message;
    };

    template <EncodeBackend EB>
    class NullEncoder : public Encoder2<EB, base::nullptr_t> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const base::nullptr_t& ptr = nullptr) const override {
            return backend.makeNull();
        }

        base::unique_ptr<Encoder2<EB, base::nullptr_t>> clone() const override {
            return base::make_unique<NullEncoder<EB>>();
        }
    };

    template <EncodeBackend EB>
    class BooleanEncoder : public Encoder2<EB, bool> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const bool& b) const override {
            return backend.makeBoolean(b);
        }

        base::unique_ptr<Encoder2<EB, bool>> clone() const override {
            return base::make_unique<BooleanEncoder<EB>>();
        }
    };

    template <EncodeBackend EB>
    class U32Encoder : public Encoder2<EB, u32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const u32& n) const override {
            return backend.makeU32(n);
        }

        base::unique_ptr<Encoder2<EB, u32>> clone() const override {
            return base::make_unique<U32Encoder<EB>>();
        }
    };

    template <EncodeBackend EB>
    class I32Encoder : public Encoder2<EB, i32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const i32& n) const override {
            return backend.makeI32(n);
        }

        base::unique_ptr<Encoder2<EB, i32>> clone() const override {
            return base::make_unique<I32Encoder<EB>>();
        }
    };

    template <EncodeBackend EB>
    class F32Encoder : public Encoder2<EB, f32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const f32& n) const override {
            return backend.makeF32(n);
        }

        base::unique_ptr<Encoder2<EB, f32>> clone() const override {
            return base::make_unique<F32Encoder<EB>>();
        }
    };

    template <EncodeBackend EB>
    class StringEncoder : public Encoder2<EB, base::string> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const base::string& s) const override {
            return backend.makeString(s);
        }

        base::unique_ptr<Encoder2<EB, base::string>> clone() const override {
            return base::make_unique<StringEncoder<EB>>();
        }
    };

    template <EncodeBackend EB, typename T>
    class ArrayEncoder : public Encoder2<EB, base::vector<T>> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        ArrayEncoder(base::unique_ptr<Encoder2<EB, T>>&& encoder) : _encoder(base::move(encoder)) {}

        base::Either<DataType, ErrorType> encode(
            EB& backend, const base::vector<T>& v) const override {
            base::vector<DataType> array;
            for (auto& item : v) {
                auto result = _encoder->encode(backend, item);
                if (result.isRight()) return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }
            return backend.makeArray(base::move(array));
        }

        base::unique_ptr<Encoder2<EB, base::vector<T>>> clone() const override {
            return base::make_unique<ArrayEncoder>(_encoder->clone());
        }

    private:
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <EncodeBackend EB, typename T>
    class FieldEncoder {
    public:
        virtual ~FieldEncoder() = default;

        virtual base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const = 0;

        virtual base::unique_ptr<FieldEncoder> clone() const = 0;

        template <typename S, typename F /* S -> T */>
        RecordEncoderBuilder<EB, S, T, F> forGetter(F getter) const {
            return RecordEncoderBuilder<EB, S, T, F>(clone(), getter);
        }
    };

    template <EncodeBackend EB, typename T>
    class NormalFieldEncoder : public FieldEncoder<EB, T> {
    public:
        NormalFieldEncoder(const base::string& name, base::unique_ptr<Encoder2<EB, T>> encoder)
            : _name(name), _encoder(base::move(encoder)) {}

        base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const {
            auto result = _encoder->encode(backend, object);
            if (result.isRight()) {
                return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
            } else {
                builder.add(_name, base::move(result.left()));
                return base::nullopt;
            }
        }

        base::unique_ptr<FieldEncoder<EB, T>> clone() const {
            return base::make_unique<NormalFieldEncoder<EB, T>>(_name, _encoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <EncodeBackend EB, typename T>
    class OptionalFieldEncoder : public FieldEncoder<EB, base::Optional<T>> {
    public:
        OptionalFieldEncoder(const base::string& name, base::unique_ptr<Encoder2<EB, T>> encoder)
            : _name(name), _encoder(base::move(encoder)) {}

        base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const base::Optional<T>& object) const {
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

        base::unique_ptr<FieldEncoder<EB, base::Optional<T>>> clone() const {
            return base::make_unique<OptionalFieldEncoder<EB, T>>(_name, _encoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <EncodeBackend EB, typename S, typename T, typename F /* S -> T */>
    class RecordEncoderBuilder {
    public:
        RecordEncoderBuilder(base::unique_ptr<FieldEncoder<EB, T>>&& field_encoder, F getter)
            : field_encoder(base::move(field_encoder)), getter(getter) {}

        RecordEncoderBuilder(const RecordEncoderBuilder& that) : field_encoder(that.field_encoder->clone()), getter(that.getter) {}

    public:
        base::unique_ptr<FieldEncoder<EB, T>> field_encoder;
        F getter;
    };

    namespace _encoder_record {

        template <EncodeBackend EB, typename S, typename... Builders>
        base::Optional<typename EB::ErrorType> encodeRecord(EB& backend, RecordBuilder<EB>& record_builder, const S& object, const Builders&... builders) {
            base::Optional<typename EB::ErrorType> error;
            // magic fold expression that allows fast failing
            bool success = ([&]() {
                auto result = builders.field_encoder->encode(backend, record_builder, builders.getter(object));
                if (result.hasValue()) {
                    error = base::move(result);
                    // This will cause the execution to fail fast, preventing encoding further entries.
                    return false;
                } else {
                    return true;
                }
            }() && ...);

            return success ? base::nullopt : error;
        }

    };  // namespace _encoder_record

    template <EncodeBackend EB, typename S, typename... Builders>
    class RecordEncoder : public Encoder2<EB, S> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        // FIXME: workaround
        // It seems that we cannot partially infer template class type parameters from constructor argument types.
        template <typename... Builders2>
        static RecordEncoder<EB, S, Builders2...> group(const Builders2&... builders) {
            return RecordEncoder<EB, S, Builders2...>(builders...);
        }

        RecordEncoder(const Builders&... builders) : _builders(builders...) {}

        base::Either<DataType, ErrorType> encode(EB& backend, const S& object) const override {
            RecordBuilderImpl<EB> record_builder;
            auto optional = std::apply(
                [&](const Builders&... builders) {
                    return _encoder_record::encodeRecord(backend, record_builder, object, builders...);
                },
                _builders);
            if (optional.hasValue()) {
                return base::Right(base::move(optional.value()));
            } else {
                return record_builder.build(backend);
            }
        }

        base::unique_ptr<Encoder2<EB, S>> clone() const override {
            return std::apply([](const Builders&... builders) { return base::make_unique<RecordEncoder>(builders...); }, _builders);
        }

    private:
        std::tuple<Builders...> _builders;
    };

    template <EncodeBackend EB, DecodeBackend DB>
    struct CodecBackend {
        EB encodeBackend;
        DB decodeBackend;
    };

}  // namespace spargel::codec
