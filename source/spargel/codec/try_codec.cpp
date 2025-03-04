
#include <spargel/base/assert.h>
#include <spargel/base/hash_map.h>
#include <spargel/base/meta.h>
#include <spargel/base/optional.h>
#include <spargel/base/string.h>
#include <spargel/base/vector.h>

namespace spargel::base {

    template <class From, class To>
    concept ConvertibleTo = base::is_convertible<From, To>;

}

namespace spargel::codec {

    /*
     * T:   object type
     * BT:  backend type
     * EB:  encode backend
     */

    /*
     * Encoder API
     */

    template <typename EB, typename BT>
    concept IsEncodeBackendOf =
        requires(EB& backend, int n, const base::string& s, const base::vector<BT>& array) {
            backend.makeInteger(n);
            { backend.makeInteger(n) } -> base::ConvertibleTo<BT>;

            backend.makeString(s);
            { backend.makeString(s) } -> base::ConvertibleTo<BT>;

            backend.makeArray(array);
            { backend.makeArray(array) } -> base::ConvertibleTo<BT>;
        };

    struct DummyBackendType {};

    struct DummyEncodeBackend {
        DummyBackendType makeInteger(int n) { return {}; }
        DummyBackendType makeString(const base::string& s) { return {}; }
        DummyBackendType makeArray(const base::vector<DummyBackendType>& v) { return {}; }
    };
    static_assert(IsEncodeBackendOf<DummyEncodeBackend, DummyBackendType>);

    /*
     * Codec API
     */

    template <typename Codec, typename T>
    concept IsCodecOf = requires(const T& object, DummyEncodeBackend& backend) {
        // FIXME?
        Codec::template encode<DummyBackendType>(object, backend);

        {
            Codec::template encode<DummyBackendType>(object, backend)
        } -> base::ConvertibleTo<base::Optional<DummyBackendType>>;
    };

    // codecs of primitive types

    struct CodecOfInteger {
        using type = int;

        template <typename BT, typename EB>
            requires IsEncodeBackendOf<EB, BT>
        static base::Optional<BT> encode(int n, EB& backend) {
            return base::makeOptional<BT>(backend.makeInteger(n));
        }
    };
    static_assert(IsCodecOf<CodecOfInteger, int>);

    struct CodecOfString {
        using type = base::string;

        template <typename BT, typename EB>
            requires IsEncodeBackendOf<EB, BT>
        static base::Optional<BT> encode(const base::string& s, EB& backend) {
            return base::makeOptional<BT>(backend.makeString(s));
        }
    };
    static_assert(IsCodecOf<CodecOfString, base::string>);

    template <typename Codec>
    struct ArrayCodecOf {
        using type = base::vector<typename Codec::type>;

        template <typename BT, typename EB>
            requires IsEncodeBackendOf<EB, BT>
        static base::Optional<BT> encode(const base::vector<typename Codec::type> array,
                                         EB& backend) {
            base::vector<BT> v;
            for (auto& item : array) {
                base::Optional<BT> result = Codec::template encode<BT>(item, backend);
                if (!result.hasValue()) return base::Optional<BT>();
                v.push(result.value());
            }
            return base::makeOptional<BT>(backend.makeArray(v));
        }
    };
    static_assert(IsCodecOf<ArrayCodecOf<CodecOfInteger>, base::vector<int>>);
    static_assert(IsCodecOf<ArrayCodecOf<CodecOfString>, base::vector<base::string>>);
    static_assert(
        IsCodecOf<ArrayCodecOf<ArrayCodecOf<CodecOfInteger>>, base::vector<base::vector<int>>>);

    /*
     * Example implementation of Encoder
     */

    struct A {
        base::vector<int> values;

        struct Codec {
            using type = A;

            template <typename BT, typename EB>
                requires IsEncodeBackendOf<EB, BT>
            static base::Optional<BT> encode(const A& a, EB& backend) {
                return ArrayCodecOf<CodecOfInteger>::encode<BT>(a.values, backend);
            }
        };
    };
    static_assert(IsCodecOf<A::Codec, A>);

    /*
     * Example implementation of Backend
     */

    struct ExampleBackendType {
        int v;
    };

    struct ExampleEncodeBackend {
        ExampleBackendType makeInteger(int n) { return {n}; }

        ExampleBackendType makeString(const base::string& s) { return {(int)s.length()}; }

        ExampleBackendType makeArray(const base::vector<ExampleBackendType>& array) {
            int v = 0;
            for (auto& item : array) v += item.v;
            return {v};
        }
    };
}  // namespace spargel::codec

int main() {
    using spargel::base::move;
    using spargel::base::string;
    using spargel::base::vector;
    using namespace spargel::codec;

    ExampleEncodeBackend backend;
    spargel::base::Optional<ExampleBackendType> result;

    result = CodecOfInteger::encode<ExampleBackendType>(11, backend);
    spargel_assert(result.hasValue() && result.value().v == 11);

    result = CodecOfString::encode<ExampleBackendType>(string("ABC"), backend);
    spargel_assert(result.hasValue() && result.value().v == 3);

    vector<int> array;
    array.push(1);
    array.push(2);
    array.push(3);
    result = ArrayCodecOf<CodecOfInteger>::encode<ExampleBackendType>(move(array), backend);
    spargel_assert(result.hasValue() && result.value().v == 1 + 2 + 3);

    vector<vector<int>> v;
    vector<int> v1;
    v1.push(1);
    v1.push(2);
    vector<int> v2;
    v2.push(3);
    v2.push(4);
    v.push(move(v1));
    v.push(move(v2));
    result =
        ArrayCodecOf<ArrayCodecOf<CodecOfInteger>>::encode<ExampleBackendType>(move(v), backend);
    spargel_assert(result.hasValue() && result.value().v == (1 + 2) + (3 + 4));

    A a;
    a.values.push(3);
    a.values.push(2);
    a.values.push(1);
    result = A::Codec::encode<ExampleBackendType>(a, backend);
    spargel_assert(result.hasValue() && result.value().v == 3 + 2 + 1);

    return 0;
}
