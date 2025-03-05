#pragma once

#include <spargel/base/concept.h>
#include <spargel/base/either.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>

namespace spargel::codec {

    // generic codec error class
    class CodecError {
    public:
        CodecError(const base::string& message) : _message(message) {}

        CodecError(const char* message) : _message(message) {}

        const base::string& message() { return _message; }

        friend CodecError operator+(const CodecError& error1, const CodecError& error2) {
            return CodecError(error1._message + error2._message);
        }

    private:
        base::string _message;
    };

    /*
     * Generic Encode/Decode API
     */

    // encode backend prototype
    template <typename EB /* encode backend type */>
    concept EncodeBackend = requires {
        typename EB::DataType;  /* backend data type */
        typename EB::ErrorType; /* encode error type */
    } && requires(EB& backend, bool b) {
        backend.makeBoolean(b);
        {
            backend.makeBoolean(b)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    } && requires(EB& backend, u32 n) {
        backend.makeU32(n);
        {
            backend.makeU32(n)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    } && requires(EB& backend, i32 n) {
        backend.makeI32(n);
        {
            backend.makeI32(n)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    } && requires(EB& backend, const base::string& s) {
        backend.makeString(s);
        {
            backend.makeString(s)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    } && requires(EB& backend, const base::vector<typename EB::DataType>& array) {
        backend.makeArray(array);
        {
            backend.makeArray(array)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    };

    // decode backend prototype
    template <typename DB /* decode backend type */>
    concept DecodeBackend = requires {
        typename DB::DataType;  /* backend data type */
        typename DB::ErrorType; /* decode error type */
    } && requires(DB& backend, const DB::DataType& data) {
        backend.getBoolean(data);
        {
            backend.getBoolean(data)
        } -> base::ConvertibleTo<base::Either<bool, typename DB::ErrorType>>;

        backend.getU32(data);
        { backend.getU32(data) } -> base::ConvertibleTo<base::Either<u32, typename DB::ErrorType>>;
        backend.getI32(data);
        { backend.getI32(data) } -> base::ConvertibleTo<base::Either<i32, typename DB::ErrorType>>;

        backend.getString(data);
        {
            backend.getString(data)
        } -> base::ConvertibleTo<base::Either<base::string, typename DB::ErrorType>>;

        backend.getArray(data);
        {
            backend.getArray(data)
        } -> base::ConvertibleTo<
              base::Either<base::vector<typename DB::DataType>, typename DB::ErrorType>>;
    };

    // dummy encode/decode backend (also used for constraint checking)
    namespace {

        struct DummyType {};

        struct EncodeBackendDummy {
            using DataType = DummyType;
            using ErrorType = CodecError;

            base::Either<DummyType, CodecError> makeBoolean(bool b);

            base::Either<DummyType, CodecError> makeU32(u32 n);
            base::Either<DummyType, CodecError> makeI32(i32 n);

            base::Either<DummyType, CodecError> makeString(const base::string& s);

            base::Either<DummyType, CodecError> makeArray(const base::vector<DummyType>& array);
        };
        static_assert(EncodeBackend<EncodeBackendDummy>);

        struct DecodeBackendDummy {
            using DataType = DummyType;
            using ErrorType = CodecError;

            base::Either<bool, CodecError> getBoolean(const DummyType& data);

            base::Either<u32, CodecError> getU32(const DummyType& data);
            base::Either<i32, CodecError> getI32(const DummyType& data);

            base::Either<base::string, CodecError> getString(const DummyType& data);

            base::Either<base::vector<DummyType>, CodecError> getArray(const DummyType& data);
        };
        static_assert(DecodeBackend<DecodeBackendDummy>);

    }  // namespace

    // encoder prototype
    template <typename E>
    concept Encoder = requires { typename E::Type; } && requires(EncodeBackendDummy& backend,
                                                                 const E::Type& object) {
        E::template encode<EncodeBackendDummy>(backend, object);
        {
            E::template encode<EncodeBackendDummy>(backend, object)
        } -> base::ConvertibleTo<
              base::Either<EncodeBackendDummy::DataType, EncodeBackendDummy::ErrorType>>;
    };

    // decoder prototype
    template <typename D>
    concept Decoder = requires {
        typename D::Type;
    } && requires(DecodeBackendDummy& backend, const DecodeBackendDummy::DataType& data) {
        D::template decode<DecodeBackendDummy>(backend, data);
        {
            D::template decode<DecodeBackendDummy>(backend, data)
        } -> base::ConvertibleTo<base::Either<typename D::Type, DecodeBackendDummy::ErrorType>>;
    };

    /*
     * encoding/decoding of basic types
     */

    struct CodecBoolean {
        using Type = bool;

        template <EncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend,
                                                                                  bool b) {
            return backend.makeBoolean(b);
        }

        template <DecodeBackend DB>
        static base::Either<bool, typename DB::ErrorType> decode(
            DB& backend, const typename DB::DataType& data) {
            return backend.getBoolean(data);
        }
    };

    struct CodecU32 {
        using Type = u32;

        template <EncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend,
                                                                                  u32 n) {
            return backend.makeU32(n);
        }

        template <DecodeBackend DB>
        static base::Either<u32, typename DB::ErrorType> decode(DB& backend,
                                                                const typename DB::DataType& data) {
            return backend.getU32(data);
        }
    };

    struct CodecI32 {
        using Type = i32;

        template <EncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(EB& backend,
                                                                                  i32 n) {
            return backend.makeI32(n);
        }

        template <DecodeBackend DB>
        static base::Either<i32, typename DB::ErrorType> decode(DB& backend,
                                                                const typename DB::DataType& data) {
            return backend.getI32(data);
        }
    };

    struct CodecString {
        using Type = base::string;

        template <EncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const base::string& s) {
            return backend.makeString(s);
        }

        template <DecodeBackend DB>
        static base::Either<base::string, typename DB::ErrorType> decode(
            DB& backend, const typename DB::DataType& data) {
            return backend.getString(data);
        }
    };

    template <typename Codec>
    struct CodecArray {
        using ChildType = Codec::Type;
        using Type = base::vector<ChildType>;

        template <EncodeBackend EB>
            requires Encoder<Codec>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            EB& backend, const Type& array) {
            base::vector<typename EB::DataType> tmp;
            for (const auto& item : array) {
                auto result = Codec::template encode(backend, item);
                if (result.isRight()) return result;
                tmp.push(base::move(result.left()));
            }

            return backend.makeArray(base::move(tmp));
        }

        template <DecodeBackend DB>
            requires Decoder<Codec>
        static base::Either<Type, typename DB::ErrorType> decode(
            DB& backend, const typename DB::DataType& data) {
            using ErrorType = DB::ErrorType;

            auto result_array = backend.getArray(data);
            if (result_array.isRight())
                return base::makeRight<Type, ErrorType>(result_array.right());

            Type tmp;
            for (const auto& item : result_array.left()) {
                auto result = Codec::template decode<DB>(backend, item);
                if (result.isRight()) return base::makeRight<Type, ErrorType>(result.right());
                tmp.push(base::move(result.left()));
            }

            return base::makeLeft<Type, ErrorType>(base::move(tmp));
        }
    };

    /**
     * todo:
     *  - bmp
     *  - png
     *  - openexr
     *  - ptex
     */

    /* this is a hack */
    struct color4 {
        u8 b;
        u8 g;
        u8 r;
        u8 a;
    };

    struct image {
        int width;
        int height;
        struct color4* pixels;
    };

    void destroy_image(struct image image);

    enum decode_result {
        DECODE_SUCCESS,
        /* todo: better error codes */
        DECODE_FAILED,
    };

    /**
     * @brief ppm loader
     */
    int load_ppm_image(char const* path, struct image* image);

    void destroy_image(struct image const* image);

}  // namespace spargel::codec
