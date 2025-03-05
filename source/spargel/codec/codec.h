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
    concept IsEncodeBackend = requires {
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
    concept IsDecodeBackend = requires {
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

        struct DummyEncodeBackend {
            using DataType = DummyType;
            using ErrorType = CodecError;

            base::Either<DummyType, CodecError> makeBoolean(bool b);

            base::Either<DummyType, CodecError> makeU32(u32 n);
            base::Either<DummyType, CodecError> makeI32(i32 n);

            base::Either<DummyType, CodecError> makeString(const base::string& s);

            base::Either<DummyType, CodecError> makeArray(const base::vector<DummyType>& array);
        };
        static_assert(IsEncodeBackend<DummyEncodeBackend>);

        struct DummyDecodeBackend {
            using DataType = DummyType;
            using ErrorType = CodecError;

            base::Either<bool, CodecError> getBoolean(const DummyType& data);

            base::Either<u32, CodecError> getU32(const DummyType& data);
            base::Either<i32, CodecError> getI32(const DummyType& data);

            base::Either<base::string, CodecError> getString(const DummyType& data);

            base::Either<base::vector<DummyType>, CodecError> getArray(const DummyType& data);
        };
        static_assert(IsDecodeBackend<DummyDecodeBackend>);

    }  // namespace

    // encoder prototype
    template <typename Encoder>
    concept IsEncoder = requires {
        typename Encoder::Type;
    } && requires(const Encoder::Type& object, DummyEncodeBackend& backend) {
        Encoder::template encode<DummyEncodeBackend>(object, backend);
        {
            Encoder::template encode<DummyEncodeBackend>(object, backend)
        } -> base::ConvertibleTo<
              base::Either<DummyEncodeBackend::DataType, DummyEncodeBackend::ErrorType>>;
    };

    // decoder prototype
    template <typename Decoder>
    concept IsDecoder = requires {
        typename Decoder::Type;
    } && requires(const DummyDecodeBackend::DataType& data, DummyDecodeBackend& backend) {
        Decoder::template decode<DummyDecodeBackend>(data, backend);
        {
            Decoder::template decode<DummyDecodeBackend>(data, backend)
        }
        -> base::ConvertibleTo<base::Either<typename Decoder::Type, DummyDecodeBackend::ErrorType>>;
    };

    /*
     * encoding/decoding of basic types
     */

    struct CodecBoolean {
        using Type = bool;

        template <IsEncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(bool b,
                                                                                  EB& backend) {
            return backend.makeBoolean(b);
        }

        template <IsDecodeBackend DB>
        static base::Either<bool, typename DB::ErrorType> decode(const typename DB::DataType& data,
                                                                 DB& backend) {
            return backend.getBoolean(data);
        }
    };
    static_assert(IsEncoder<CodecBoolean> && IsDecoder<CodecBoolean>);

    struct CodecU32 {
        using Type = u32;

        template <IsEncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(u32 n,
                                                                                  EB& backend) {
            return backend.makeU32(n);
        }

        template <IsDecodeBackend DB>
        static base::Either<u32, typename DB::ErrorType> decode(const typename DB::DataType& data,
                                                                DB& backend) {
            return backend.getU32(data);
        }
    };
    static_assert(IsEncoder<CodecU32> && IsDecoder<CodecU32>);

    struct CodecI32 {
        using Type = i32;

        template <IsEncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(i32 n,
                                                                                  EB& backend) {
            return backend.makeI32(n);
        }

        template <IsDecodeBackend DB>
        static base::Either<i32, typename DB::ErrorType> decode(const typename DB::DataType& data,
                                                                DB& backend) {
            return backend.getI32(data);
        }
    };
    static_assert(IsEncoder<CodecI32> && IsDecoder<CodecI32>);

    struct CodecString {
        using Type = base::string;

        template <IsEncodeBackend EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const base::string& s, EB& backend) {
            return backend.makeString(s);
        }

        template <IsDecodeBackend DB>
        static base::Either<base::string, typename DB::ErrorType> decode(
            const typename DB::DataType& data, DB& backend) {
            return backend.getString(data);
        }
    };
    static_assert(IsEncoder<CodecString> && IsDecoder<CodecString>);

    template <typename Codec>
    struct CodecArray {
        using ChildType = Codec::Type;
        using Type = base::vector<ChildType>;

        template <IsEncodeBackend EB>
            requires IsEncoder<Codec>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(
            const base::vector<ChildType>& array, EB& backend) {
            using DataType = EB::DataType;
            using ErrorType = EB::ErrorType;

            base::vector<DataType> tmp;
            for (const auto& item : array) {
                base::Either<DataType, ErrorType> result =
                    Codec::template encode<EB>(item, backend);
                if (result.isRight()) return result;
                tmp.push(base::move(result.left()));
            }

            return backend.makeArray(base::move(tmp));
        }

        template <IsDecodeBackend DB>
            requires IsDecoder<Codec>
        static base::Either<base::vector<ChildType>, typename DB::ErrorType> decode(
            const typename DB::DataType& data, DB& backend) {
            using DataType = DB::DataType;
            using ErrorType = DB::ErrorType;

            base::Either<base::vector<DataType>, ErrorType> result_array = backend.getArray(data);
            if (result_array.isRight())
                return base::makeRight<base::vector<ChildType>, ErrorType>(result_array.right());

            base::vector<ChildType> tmp;
            for (const auto& item : result_array.left()) {
                base::Either<ChildType, ErrorType> result =
                    Codec::template decode<DB>(item, backend);
                if (result.isRight())
                    return base::makeRight<base::vector<ChildType>, ErrorType>(result.right());
                tmp.push(base::move(result.left()));
            }

            return base::makeLeft<base::vector<ChildType>, ErrorType>(base::move(tmp));
        }
    };
    static_assert(IsEncoder<CodecArray<CodecI32>>);
    static_assert(IsEncoder<CodecArray<CodecString>>);
    static_assert(IsDecoder<CodecArray<CodecI32>>);
    static_assert(IsDecoder<CodecArray<CodecString>>);
    static_assert(IsEncoder<CodecArray<CodecArray<CodecI32>>>);
    static_assert(IsDecoder<CodecArray<CodecArray<CodecString>>>);

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
