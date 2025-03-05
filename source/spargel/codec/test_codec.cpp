#include <spargel/codec/codec.h>

using namespace spargel;
using namespace spargel::codec;

namespace {

    struct TestData {
        int value;
    };

    struct TestEncodeBackend {
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

    static_assert(IsEncodeBackend<TestEncodeBackend>);

}  // namespace

void test_encode_primitive() {
    TestEncodeBackend backend;
    base::Either<TestData, CodecError> result = base::makeRight<TestData, CodecError>("");

    result = CodecBoolean::encode(true, backend);
    spargel_assert(result.isLeft() && result.left().value == 1);
    result = CodecBoolean::encode(false, backend);
    spargel_assert(result.isLeft() && result.left().value == 0);

    result = CodecU32::encode(123, backend);
    spargel_assert(result.isLeft() && result.left().value == 123);
    result = CodecI32::encode(-321, backend);
    spargel_assert(result.isLeft() && result.left().value == -321);

    result = CodecString::encode(base::string("ABC"), backend);
    spargel_assert(result.isLeft() && result.left().value == 3);

    base::vector<base::string> v1;
    v1.push("A");
    v1.push("BC");
    v1.push("DEF");
    result = CodecArray<CodecString>::encode(base::move(v1), backend);
    spargel_assert(result.isLeft() && result.left().value == 1 + 2 + 3);

    base::vector<base::vector<i32>> vv1;
    base::vector<i32> v11;
    v11.push(1);
    v11.push(2);
    vv1.push(base::move(v11));
    base::vector<i32> v12;
    v12.push(-3);
    v12.push(-4);
    vv1.push(base::move(v12));
    result = CodecArray<CodecArray<CodecI32>>::encode(base::move(vv1), backend);
    spargel_assert(result.isLeft() && result.left().value == (1 + 2) + (-3 + -4));
}

int main() { return 0; }
