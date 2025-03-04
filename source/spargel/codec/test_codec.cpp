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

    result = CodecOfString::encode(base::string("ABC"), backend);
    spargel_assert(result.isLeft());
    spargel_assert(result.left().value == 3);

    base::vector<base::string> v1;
    v1.push("A");
    v1.push("BC");
    v1.push("DEF");
    result = ArrayCodecOf<CodecOfString>::encode(base::move(v1), backend);
    spargel_assert(result.isLeft());
    spargel_assert(result.left().value == 1 + 2 + 3);

    base::vector<base::vector<base::string>> vv1;
    base::vector<base::string> v11;
    v11.push("A");
    v11.push("BC");
    vv1.push(base::move(v11));
    base::vector<base::string> v12;
    v12.push("DEF");
    v12.push("GHIJ");
    vv1.push(base::move(v12));
    result = ArrayCodecOf<ArrayCodecOf<CodecOfString>>::encode(base::move(vv1), backend);
    spargel_assert(result.isLeft());
    spargel_assert(result.left().value == (1 + 2) + (3 + 4));
}

int main() { return 0; }
