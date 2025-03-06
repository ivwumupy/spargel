#pragma once

#include <spargel/base/concept.h>
#include <spargel/base/either.h>
#include <spargel/base/functional.h>
#include <spargel/base/hash_map.h>
#include <spargel/base/optional.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>

namespace spargel::codec {

    /*
     * generic codec error class
     *
     * Error classes for codec API can be easily built base upon this, since they are
     * required to be constructable from error message.
     */
    class CodecError {
    public:
        CodecError(const base::string& message) : _message(message) {}

        CodecError(const base::string_view& message) : _message(message) {}

        CodecError(const char* message) : _message(message) {}

        const base::string& message() { return _message; }

        friend CodecError operator+(const CodecError& error1, const CodecError& error2) {
            return CodecError(error1._message + error2._message);
        }

    private:
        base::string _message;
    };

    /*
     * Generic Codec (Encode/Decode) API
     */

    // We allow codec backend to change itself during operations, so we do not use "const"
    // qualifier.

    // Error classes for codec API should be directly constructable from messages.
    template <typename E>
    concept ConstructableFromMessage =
        requires(const base::string& s) { E(s); } && requires(const base::string_view& s) { E(s); };

    // encode backend prototype
    template <typename EB /* encode backend type */>
    concept EncodeBackend = requires {
        typename EB::DataType;  /* backend data type */
        typename EB::ErrorType; /* encode error type */
        requires ConstructableFromMessage<typename EB::ErrorType>;
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
    } && requires(EB& backend, const base::HashMap<base::string, typename EB::DataType>& map) {
        backend.makeMap(map);
        {
            backend.makeMap(map)
        } -> base::ConvertibleTo<base::Either<typename EB::DataType, typename EB::ErrorType>>;
    };

    // decode backend prototype
    template <typename DB /* decode backend type */>
    concept DecodeBackend = requires {
        typename DB::DataType;  /* backend data type */
        typename DB::ErrorType; /* decode error type */
        requires ConstructableFromMessage<typename DB::ErrorType>;
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
    } && requires(DB& backend, const DB::DataType& data, const base::string& key) {
        backend.getMember(data, key);
        {
            backend.getMember(data, key)
        } -> base::ConvertibleTo<
              base::Either<base::Optional<typename DB::DataType>, typename DB::ErrorType>>;
    };

    // dummy encode/decode backend
    //   used for constraint checking
    //   also serves as an example
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

            base::Either<DummyType, CodecError> makeMap(
                const base::HashMap<base::string, DummyType>& map);
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

            base::Either<base::Optional<DummyType>, CodecError> getMember(const DummyType& data,
                                                                          const base::string& key);
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
     * codecs of basic types
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
                auto result = Codec::template encode<EB>(backend, item);
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

    /*
     * utils for encoding maps
     */

    namespace __encode_map {

        struct EncodeMapEntry {
            // the entry will always be encoded to the target map
            template <Decoder E>
            struct Normal {
                base::string key;
                E::Type value;

                Normal(const base::string& key, const E::Type value) : key(key), value(value) {}
                Normal(const base::string_view& key, const E::Type value)
                    : key(key), value(value) {}
            };

            // the entry will be encoded to the taregt map only if it exists
            template <Decoder E>
            struct Optional {
                base::string key;
                base::Optional<typename E::Type> value;

                Optional(const base::string& key, const base::Optional<typename E::Type>& value)
                    : key(key), value(value) {}
                Optional(const base::string_view& key,
                         const base::Optional<typename E::Type>& value)
                    : key(key), value(value) {}
            };
        };

        // head for recursive definition: no entries at all
        template <EncodeBackend EB>
        base::Optional<typename EB::ErrorType> encodeToMap(
            base::HashMap<base::string, typename EB::DataType>& map, EB& backend) {
            return base::Optional<typename EB::ErrorType>();
        }

        // encode one normal entry
        template <EncodeBackend EB, Encoder E, typename... Entries>
        base::Optional<typename EB::ErrorType> encodeToMap(
            base::HashMap<base::string, typename EB::DataType>& map, EB& backend,
            EncodeMapEntry::Normal<E> entry, Entries... entries) {
            auto result = E::template encode<EB>(backend, entry.value);
            if (result.isRight()) return base::makeOptional<typename EB::ErrorType>(result.right());

            map.set(entry.key, result.left());

            return encodeToMap(map, backend, base::forward<Entries>(entries)...);
        }

        // encode one optional entry
        template <EncodeBackend EB, Encoder E, typename... Entries>
        base::Optional<typename EB::ErrorType> encodeToMap(
            base::HashMap<base::string, typename EB::DataType>& map, EB& backend,
            EncodeMapEntry::Optional<E> entry, Entries... entries) {
            if (entry.value.hasValue()) {
                auto result = E::template encode<EB>(backend, entry.value.value());
                if (result.isRight())
                    return base::makeOptional<typename EB::ErrorType>(result.right());

                map.set(entry.key, result.left());
            }

            return encodeToMap(map, backend, base::forward<Entries>(entries)...);
        }

        // create map, call encodeToMap to encode all the entries, and return the map
        template <EncodeBackend EB, typename... Encoders, typename... Entries>
        base::Either<typename EB::DataType, typename EB::ErrorType> encodeMap(EB& backend,
                                                                              Entries... entries) {
            using DataType = EB::DataType;
            using ErrorType = EB::ErrorType;

            base::HashMap<base::string, DataType> map(base::default_allocator());  // FIXME
            auto result =
                encodeToMap<Encoders...>(map, backend, base::forward<Entries>(entries)...);
            if (result.hasValue()) return base::makeRight<DataType, ErrorType>(result.value());
            return backend.makeMap(base::move(map));
        }

    }  // namespace __encode_map

    using __encode_map::EncodeMapEntry;

    using __encode_map::encodeMap;

    /*
     * utils for decoding maps
     */

    namespace __decode_map {

        struct DecodeMapEntry {
            // the entry is required to exist
            template <Decoder D>
            struct Required {
                base::string key;

                Required(const base::string& key) : key(key) {}
                Required(const base::string_view& key) : key(key) {}
            };

            // the entry is optional
            template <Decoder D>
            struct Optional {
                base::string key;

                Optional(const base::string& key) : key(key) {}
                Optional(const base::string_view& key) : key(key) {}
            };

            // the entry is optional; if it does not exist, supply a default value as the decode
            // result
            template <Decoder D>
            struct Default {
                base::string key;
                D::Type default_value;

                Default(const base::string& key, const D::Type& default_value)
                    : key(key), default_value(default_value) {}

                Default(const base::string_view& key, const D::Type& default_value)
                    : key(key), default_value(default_value) {}
            };
        };

        // head for recursive definition: no entries at all
        template <typename R, typename F, DecodeBackend DB>
        base::Either<R, typename DB::ErrorType> decodeMap(F func, DB& backend,
                                                          const typename DB::DataType& data) {
            return base::makeLeft<R, typename DB::ErrorType>(func());
        }

        // decode one required entry
        template <typename R, typename F, DecodeBackend DB, Decoder D, typename... Entries>
        base::Either<R, typename DB::ErrorType> decodeMap(F func, DB& backend,
                                                          const typename DB::DataType& data,
                                                          DecodeMapEntry::Required<D> entry,
                                                          Entries... entries) {
            auto result = backend.getMember(data, entry.key);
            if (result.isRight()) return base::makeRight<R, typename DB::ErrorType>(result.right());

            auto optional = result.left();
            if (optional.hasValue()) {
                auto decode_result = D::template decode<DB>(backend, base::move(optional.value()));
                if (decode_result.isRight())
                    return base::makeRight<R, typename DB::ErrorType>(decode_result.right());

                return decodeMap<R>(curry(func, base::move(decode_result.left())), backend, data,
                                    base::forward<Entries>(entries)...);
            } else {
                return base::makeRight<R, typename DB::ErrorType>(
                    base::string("required member \"") + entry.key + "\" not found");
            }
        }

        // decode one optional entry
        template <typename R, typename F, DecodeBackend DB, Decoder D, typename... Entries>
        base::Either<R, typename DB::ErrorType> decodeMap(F func, DB& backend,
                                                          const typename DB::DataType& data,
                                                          DecodeMapEntry::Optional<D> entry,
                                                          Entries... entries) {
            auto result = backend.getMember(data, entry.key);
            if (result.isRight()) return base::makeRight<R, typename DB::ErrorType>(result.right());

            auto optional = result.left();
            if (optional.hasValue()) {
                auto decode_result = D::template decode<DB>(backend, base::move(optional.value()));
                if (decode_result.isRight())
                    return base::makeRight<R, typename DB::ErrorType>(decode_result.right());

                return decodeMap<R>(curry(func, base::makeOptional<typename D::Type>(
                                                    base::move(decode_result.left()))),
                                    backend, data, base::forward<Entries>(entries)...);
            } else {
                return decodeMap<R>(curry(func, base::Optional<typename D::Type>()), backend, data,
                                    base::forward<Entries>(entries)...);
            }
        }

        // decode one optional entry with default value
        template <typename R, typename F, DecodeBackend DB, Decoder D, typename... Entries>
        base::Either<R, typename DB::ErrorType> decodeMap(F func, DB& backend,
                                                          const typename DB::DataType& data,
                                                          DecodeMapEntry::Default<D> entry,
                                                          Entries... entries) {
            auto result = backend.getMember(data, entry.key);
            if (result.isRight()) return base::makeRight<R, typename DB::ErrorType>(result.right());

            auto optional = result.left();
            if (optional.hasValue()) {
                auto decode_result = D::template decode<DB>(backend, base::move(optional.value()));
                if (decode_result.isRight())
                    return base::makeRight<R, typename DB::ErrorType>(decode_result.right());

                return decodeMap<R>(curry(func, base::move(decode_result.left())), backend, data,
                                    base::forward<Entries>(entries)...);
            } else {
                return decodeMap<R>(curry(func, entry.default_value), backend, data,
                                    base::forward<Entries>(entries)...);
            }
        }

    }  // namespace __decode_map

    using __decode_map::DecodeMapEntry;

    using __decode_map::decodeMap;

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
