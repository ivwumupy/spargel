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
    };

    static_assert(EncodeBackend<EncodeBackendTest>);

}  // namespace

void test_encode_primitive() {
    EncodeBackendTest encoder;
    auto result = base::makeRight<TestData, CodecError>("");

    result = CodecBoolean::encode(encoder, true);
    spargel_assert(result.isLeft() && result.left().value == 1);
    result = CodecBoolean::encode(encoder, false);
    spargel_assert(result.isLeft() && result.left().value == 0);

    result = CodecU32::encode(encoder, 123);
    spargel_assert(result.isLeft() && result.left().value == 123);
    result = CodecI32::encode(encoder, -321);
    spargel_assert(result.isLeft() && result.left().value == -321);

    result = CodecString::encode(encoder, base::string("ABC"));
    spargel_assert(result.isLeft() && result.left().value == 3);

    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        result = CodecArray<CodecString>::encode(encoder, base::move(v));
        spargel_assert(result.isLeft() && result.left().value == 1 + 2 + 3);
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
        result = CodecArray<CodecArray<CodecI32>>::encode(encoder, base::move(v));
        spargel_assert(result.isLeft() && result.left().value == (1 + -2) + (3 + -4));
    }
}

int main() { return 0; }
