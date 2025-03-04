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

            base::Either<DummyType, CodecError> makeString(const base::string& s);
            base::Either<DummyType, CodecError> makeArray(const base::vector<DummyType>& array);
        };
        static_assert(IsEncodeBackend<DummyEncodeBackend>);

        struct DummyDecodeBackend {
            using DataType = DummyType;
            using ErrorType = CodecError;

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

    struct CodecOfString {
        using Type = base::string;

        template <typename EB>
            requires IsEncodeBackend<EB>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(const Type& s,
                                                                                  EB& backend) {
            return backend.makeString(s);
        }

        template <typename DB>
            requires IsDecodeBackend<DB>
        static base::Either<Type, typename DB::ErrorType> decode(const typename DB::DataType& data,
                                                                 DB& backend) {
            return backend.getString(data);
        }
    };
    static_assert(IsEncoder<CodecOfString> && IsDecoder<CodecOfString>);

    template <typename Codec>
    struct ArrayCodecOf {
        using ChildType = Codec::Type;
        using Type = base::vector<ChildType>;

        template <typename EB>
            requires IsEncodeBackend<EB> && IsEncoder<Codec>
        static base::Either<typename EB::DataType, typename EB::ErrorType> encode(const Type& array,
                                                                                  EB& backend) {
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

        template <typename DB>
            requires IsDecodeBackend<DB> && IsDecoder<Codec>
        static base::Either<Type, typename DB::ErrorType> decode(const typename DB::DataType& data,
                                                                 DB& backend) {
            using DataType = DB::DataType;
            using ErrorType = DB::ErrorType;

            base::Either<Type, ErrorType> result_array = backend.getArray(data);
            if (result_array.isRight())
                return base::makeRight<Type, ErrorType>(result_array.right());

            Type tmp;
            for (const auto& item : result_array.left()) {
                base::Either<ChildType, ErrorType> result =
                    Codec::template decode<DataType, DB, ErrorType>(item, backend);
                if (result.isRight()) return base::makeRight<Type, ErrorType>(result.right());
                tmp.push(base::move(result.left()));
            }

            return base::makeLeft<Type, ErrorType>(base::move(tmp));
        }
    };
    static_assert(IsEncoder<ArrayCodecOf<CodecOfString>>);
    static_assert(IsDecoder<ArrayCodecOf<CodecOfString>>);
    static_assert(IsEncoder<ArrayCodecOf<ArrayCodecOf<CodecOfString>>>);
    static_assert(IsDecoder<ArrayCodecOf<ArrayCodecOf<CodecOfString>>>);

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
