#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/codec/codec.h>

using namespace spargel;
using namespace spargel::codec;

namespace {

    static_assert(Encoder<CodecBoolean> && Decoder<CodecBoolean>);
    static_assert(Encoder<CodecU32> && Decoder<CodecU32>);
    static_assert(Encoder<CodecI32> && Decoder<CodecI32>);
    static_assert(Encoder<CodecString> && Decoder<CodecString>);

    static_assert(Encoder<CodecArray<CodecI32>>);
    static_assert(Encoder<CodecArray<CodecString>>);
    static_assert(Decoder<CodecArray<CodecI32>>);
    static_assert(Decoder<CodecArray<CodecString>>);
    static_assert(Encoder<CodecArray<CodecArray<CodecString>>>);
    static_assert(Decoder<CodecArray<CodecArray<CodecI32>>>);
    static_assert(Encoder<CodecArray<CodecArray<CodecArray<CodecI32>>>>);
    static_assert(Decoder<CodecArray<CodecArray<CodecArray<CodecString>>>>);

    struct TestData {
        int value;
    };

    struct EncodeBackendTest {
        using DataType = TestData;
        using ErrorType = CodecError;

        base::Either<TestData, CodecError> makeBoolean(bool b) {
            return base::makeLeft<TestData, CodecError>(b ? 1 : 0);
        }

        base::Either<TestData, CodecError> makeU32(u32 n) {
            return base::makeLeft<TestData, CodecError>(n);
        }

        base::Either<TestData, CodecError> makeI32(i32 n) {
            return base::makeLeft<TestData, CodecError>(n);
        }

        base::Either<TestData, CodecError> makeString(const base::string& s) {
            return base::makeLeft<TestData, CodecError>((int)s.length());
        }

        base::Either<TestData, CodecError> makeArray(const base::vector<TestData>& array) {
            int sum = 0;
            for (const auto& item : array) sum += item.value;
            return base::makeLeft<TestData, CodecError>(sum);
        }

        base::Either<TestData, CodecError> makeMap(
            const base::HashMap<base::string, TestData>& map) {
            int sum = 0;
            // TODO
            return base::makeLeft<TestData, CodecError>(sum);
        }
    };

    static_assert(EncodeBackend<EncodeBackendTest>);

}  // namespace

TEST(Codec_Encode_Primitive) {
    EncodeBackendTest backend;
    auto result = base::makeRight<TestData, CodecError>("");

    result = CodecBoolean::encode(backend, true);
    spargel_check(result.isLeft() && result.left().value == 1);
    result = CodecBoolean::encode(backend, false);
    spargel_check(result.isLeft() && result.left().value == 0);

    result = CodecU32::encode(backend, 123);
    spargel_check(result.isLeft() && result.left().value == 123);
    result = CodecI32::encode(backend, -321);
    spargel_check(result.isLeft() && result.left().value == -321);

    result = CodecString::encode(backend, base::string("ABC"));
    spargel_check(result.isLeft() && result.left().value == 3);
}

TEST(Codec_Encode_Array) {
    EncodeBackendTest backend;

    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        auto result = CodecArray<CodecString>::encode(backend, base::move(v));
        spargel_check(result.isLeft() && result.left().value == 1 + 2 + 3);
    }
    {
        base::vector<base::vector<i32>> v;
        base::vector<i32> v1;
        v1.push(1);
        v1.push(-2);
        v.push(base::move(v1));
        base::vector<i32> v2;
        v2.push(3);
        v2.push(-4);
        v.push(base::move(v2));
        auto result = CodecArray<CodecArray<CodecI32>>::encode(backend, base::move(v));
        spargel_check(result.isLeft() && result.left().value == (1 + -2) + (3 + -4));
    }
}

TEST(Codec_Encode_Map) {
    EncodeBackendTest backend;

    {
        auto result = encodeMap(backend);
        spargel_check(result.isLeft());
        // TODO
    }
    {
        base::vector<u32> scores;
        scores.push(98);
        scores.push(87);
        scores.push(92);
        auto result =
            encodeMap(backend,  //
                      EncodeMapEntry::Normal<CodecString>("name", base::string("Alice")),
                      EncodeMapEntry::Normal<CodecU32>("age", 20),
                      EncodeMapEntry::Normal<CodecBoolean>("happy", true),
                      EncodeMapEntry::Normal<CodecArray<CodecU32>>("scores", base::move(scores)));
        spargel_check(result.isLeft());
        // TODO
    }
}
