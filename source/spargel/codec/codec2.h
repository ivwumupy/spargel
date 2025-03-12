/*
 * This implementation was inspired by Mojang's DataFixerUpper library.
 * Much of the design and implementation references the library.
 *
 * See LICENCE_Mojang_DataFixerUpper for original license.
 */

#pragma once

#include <spargel/base/tuple.h>
#include <spargel/codec/codec.h>

namespace spargel::codec {

    class A {
    public:
        void say() {}
    };

    template <EncodeBackend EB, typename T>
    class EncoderArray;

    template <EncodeBackend EB, typename T>
    class Encoder2 {
    public:
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

        Encoder2(EB* backend) : _backend(backend) {}

        virtual base::Either<DataType, ErrorType> encode(const T& object) const = 0;

        EncoderArray<EB, T> arrayOf() { return EncoderArray(_backend, this); }

    protected:
        EB* _backend;
    };

    template <EncodeBackend EB>
    class EncoderNull : public Encoder2<EB, base::nullptr_t> {
    public:
        EncoderNull(EB* backend) : Encoder2<EB, base::nullptr_t>(backend) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const base::nullptr_t& ptr = nullptr) const override {
            return this->_backend->makeNull();
        }
    };

    template <EncodeBackend EB>
    class EncoderBoolean : public Encoder2<EB, bool> {
    public:
        EncoderBoolean(EB* backend) : Encoder2<EB, bool>(backend) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const bool& b) const override {
            return this->_backend->makeBoolean(b);
        }
    };

    template <EncodeBackend EB>
    class EncoderI32 : public Encoder2<EB, i32> {
    public:
        EncoderI32(EB* backend) : Encoder2<EB, i32>(backend) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const i32& n) const override {
            return this->_backend->makeI32(n);
        }
    };

    template <EncodeBackend EB>
    class EncoderString : public Encoder2<EB, base::string> {
    public:
        EncoderString(EB* backend) : Encoder2<EB, base::string>(backend) {}

        base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const base::string& s) const override {
            return this->_backend->makeString(s);
        }
    };

    template <EncodeBackend EB, typename T>
    class EncoderArray : public Encoder2<EB, base::vector<T>> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        EncoderArray(EB* backend, const Encoder2<EB, T>* encoder)
            : Encoder2<EB, base::vector<T>>(backend), _encoder(encoder) {}

        base::Either<DataType, ErrorType> encode(const base::vector<T>& v) const override {
            base::vector<DataType> array;
            for (auto& item : v) {
                auto result = _encoder->encode(item);
                if (result.isRight()) return base::Right(base::move(result.right()));
                array.push(base::move(result.left()));
            }
            return this->_backend->makeArray(base::move(array));
        }

    private:
        const Encoder2<EB, T>* _encoder;
    };

    template <EncodeBackend EB, typename T, typename F, typename... Fields>
    class EncoderMap : public Encoder2<EB, T> {
        using DataType = EB::DataType;
        using ErrorType = EB::ErrorType;

    public:
        EncoderMap(EB* backend, const F& func) : Encoder2<EB, T>(backend), _func(func) {}

        base::Either<DataType, ErrorType> encode(const base::vector<T>& v) const override {}

    private:
        F _func;
    };

    template <EncodeBackend EB, DecodeBackend DB>
    struct CodecBackend {
        EB encodeBackend;
        DB decodeBackend;
    };

}  // namespace spargel::codec
