/*
 * This implementation was inspired by https://github.com/Mojang/DataFixerUpper.
 * See LICENCE_Mojang_DataFixerUpper.
 */

#pragma once

#include <spargel/base/tuple.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/codec/codec.h>

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
    class EncoderArray;
    template <EncodeBackend EB, typename S, typename T, typename F>
    class EncoderRecordBuilder;
    template <EncodeBackend EB, typename T>
    class FieldEncoderNormal;

    namespace _encoder2 {

        template <EncodeBackend EB, typename T>
        class Error : public Encoder2<EB, T> {
            using DataType = EB::DataType;
            using ErrorType = EB::ErrorType;

        public:
            Error(const base::string& message) : _message(message) {}

            base::Either<DataType, ErrorType> encode(EB& backend, const T& object) const {
                return base::Right(_message);
            }
            base::unique_ptr<Encoder2<EB, T>> clone() const {
                return base::make_unique<Error>(_message);
            }

        private:
            base::string _message;
        };

    }  // namespace _encoder2

    template <EncodeBackend EB, typename T>
    class Encoder2 {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        virtual ~Encoder2() = default;

        virtual base::Either<DataType, ErrorType> encode(EB& backend, const T& object) const = 0;

        virtual base::unique_ptr<Encoder2> clone() const = 0;

        FieldEncoderNormal<EB, T> fieldOf(base::string_view name) const {
            return FieldEncoderNormal<EB, T>(name, clone());
        }

        EncoderArray<EB, T> arrayOf() { return EncoderArray<EB, T>(clone()); }

    private:
    public:
        static base::unique_ptr<Encoder2> error(const base::string& message) {
            return base::make_unique<_encoder2::Error<EB, T>>(message);
        }
    };

    template <EncodeBackend EB>
    class EncoderNull : public Encoder2<EB, base::nullptr_t> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const base::nullptr_t& ptr = nullptr) const override {
            return backend.makeNull();
        }

        base::unique_ptr<Encoder2<EB, base::nullptr_t>> clone() const override {
            return base::make_unique<EncoderNull<EB>>(*this);
        }
    };

    template <EncodeBackend EB>
    class EncoderBoolean : public Encoder2<EB, bool> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const bool& b) const override {
            return backend.makeBoolean(b);
        }

        base::unique_ptr<Encoder2<EB, bool>> clone() const override {
            return base::make_unique<EncoderBoolean<EB>>(*this);
        }
    };

    template <EncodeBackend EB>
    class EncoderI32 : public Encoder2<EB, i32> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const i32& n) const override {
            return backend.makeI32(n);
        }

        base::unique_ptr<Encoder2<EB, i32>> clone() const override {
            return base::make_unique<EncoderI32<EB>>(*this);
        }
    };

    template <EncodeBackend EB>
    class EncoderString : public Encoder2<EB, base::string> {
    public:
        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const base::string& s) const override {
            return backend.makeString(s);
        }

        base::unique_ptr<Encoder2<EB, base::string>> clone() const override {
            return base::make_unique<EncoderString<EB>>(*this);
        }
    };

    template <EncodeBackend EB, typename T>
    class EncoderArray : public Encoder2<EB, base::vector<T>> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        EncoderArray(base::unique_ptr<Encoder2<EB, T>>&& encoder) : _encoder(base::move(encoder)) {}

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
            return base::make_unique<EncoderArray>(_encoder->clone());
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

        template <typename S, typename F>
        EncoderRecordBuilder<EB, S, T, F> forGetter(const F& getter) const {
            return EncoderRecordBuilder<EB, S, T, F>(clone(), getter);
        }
    };

    template <EncodeBackend EB, typename T>
    class FieldEncoderNormal : public FieldEncoder<EB, T> {
    public:
        FieldEncoderNormal(const base::string& name, base::unique_ptr<Encoder2<EB, T>> encoder)
            : _name(name), _encoder(base::move(encoder)) {}

        base::Optional<typename EB::ErrorType> encode(EB& backend, RecordBuilder<EB>& builder, const T& object) const {
            auto result = _encoder->encode(backend, object);
            if (result.isRight())
                return base::makeOptional<typename EB::ErrorType>(base::move(result.right()));
            builder.add(_name, base::move(result.left()));
        }

        base::unique_ptr<FieldEncoder<EB, T>> clone() const {
            return base::make_unique<FieldEncoderNormal<EB, T>>(_name, _encoder->clone());
        }

    private:
        base::string _name;
        base::unique_ptr<Encoder2<EB, T>> _encoder;
    };

    template <EncodeBackend EB, typename S, typename T, typename F>
    class EncoderRecordBuilder {
    public:
        EncoderRecordBuilder(base::unique_ptr<FieldEncoder<EB, T>>&& field_encoder, const F& getter)
            : field_encoder(base::move(field_encoder)), getter(getter) {}

        base::unique_ptr<FieldEncoder<EB, T>> field_encoder;
        const F& getter;
    };

    namespace _encoder_record {

        template <EncodeBackend EB, typename S>
        base::Optional<typename EB::ErrorType> encodeRecord(EB& backend, RecordBuilder<EB>& record_builder, const S& object) {}

        template <EncodeBackend EB, typename S, typename T, typename F, typename... Builders>
        base::Optional<typename EB::ErrorType> encodeRecord(EB& backend, RecordBuilder<EB>& record_builder, const S& object,
                                                            const EncoderRecordBuilder<EB, S, T, F>& builder, const Builders&... builders) {
            auto optional = builder.field_encoder->encode(backend, record_builder, builder.getter(object));
            if (optional.hasValue()) return optional;
            return encodeRecord(backend, record_builder, object, builders...);
        }

    };  // namespace _encoder_record

    template <EncodeBackend EB, typename S, typename... Builders>
    class EncoderRecord : public Encoder2<EB, S> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        EncoderRecord(Builders&&... builders) : _builders(base::move(builders)...) {}

        base::Either<DataType, ErrorType> encode(EB& backend, const S& object) const override {
            RecordBuilder<EB> record_builder;
            auto optional = base::apply(
                [&backend, &record_builder, &object](const Builders&... builders) {
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
        }

    private:
        base::Tuple<Builders...> _builders;
    };

    template <EncodeBackend EB, DecodeBackend DB>
    struct CodecBackend {
        EB encodeBackend;
        DB decodeBackend;
    };

}  // namespace spargel::codec
