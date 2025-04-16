#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/codec/codec2.h>

// libc
#include <math.h>

using namespace spargel;
using namespace spargel::codec;

namespace {

    struct TestData {
        u64 value;
    };

    struct EncodeBackendTest {
        using DataType = TestData;
        using ErrorType = CodecError;

        base::Either<TestData, CodecError> makeNull() {
            return base::Left(TestData(0));
        }

        base::Either<TestData, CodecError> makeBoolean(bool b) {
            return base::Left(TestData(b ? 1 : 0));
        }

        base::Either<TestData, CodecError> makeU8(u8 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeI8(i8 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeU16(u16 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeI16(i16 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeU32(u32 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeI32(i32 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeU64(u64 n) {
            return base::Left(TestData(n));
        }
        base::Either<TestData, CodecError> makeI64(i64 n) {
            return base::Left(TestData(n));
        }

        base::Either<TestData, CodecError> makeF32(f32 v) {
            return base::Left(TestData(v));
        }
        base::Either<TestData, CodecError> makeF64(f64 v) {
            return base::Left(TestData(v));
        }

        base::Either<TestData, CodecError> makeString(const base::string& s) {
            return base::Left(TestData((int)s.length()));
        }

        base::Either<TestData, CodecError> makeArray(const base::vector<TestData>& array) {
            int sum = 0;
            for (const auto& item : array) sum += item.value;
            return base::Left(TestData(sum));
        }

        base::Either<TestData, CodecError> makeMap(
            const base::HashMap<base::string, TestData>& map) {
            int sum = 0;
            // TODO
            return base::Left(TestData(sum));
        }
    };

    static_assert(EncodeBackend<EncodeBackendTest>);

    struct Student {
        base::string type = base::string("normal");
        base::string name;
        base::Optional<base::string> nickname;
        u32 age;
        bool happy;
        base::vector<f32> scores;

        template <EncodeBackend EB>
        static auto encoder() {
            return RecordEncoder<EB, Student>::group(
                StringEncoder<EB>().fieldOf("type").template forGetter<Student>([](const Student& student) { return student.type; }),
                StringEncoder<EB>().fieldOf("name").template forGetter<Student>([](const Student& student) { return student.name; }),
                StringEncoder<EB>().optionalFieldOf("nickname").template forGetter<Student>([](const Student& student) { return student.nickname; }),
                U32Encoder<EB>().fieldOf("age").template forGetter<Student>([](const Student& student) { return student.age; }),
                BooleanEncoder<EB>().fieldOf("happy").template forGetter<Student>([](const Student& student) { return student.happy; }),
                F32Encoder<EB>().arrayOf().fieldOf("scores").template forGetter<Student>([](const Student& student) { return student.scores; }));
        }
    };

    using EB = EncodeBackendTest;

    auto backend = EB();

    auto studentEncoder = Student::encoder<EB>();

}  // namespace

TEST(Codec2_Encode_Error) {
    auto result = ErrorEncoder<EB, int>("error").encode(backend, 0);
    spargel_check(result.isRight());
}

TEST(Codec2_Encode_Primitive) {
    auto result = base::makeRight<TestData, CodecError>("");

    result = NullEncoder<EB>().encode(backend, nullptr);
    spargel_check(result.isLeft() && result.left().value == 0);

    result = BooleanEncoder<EB>().encode(backend, true);
    spargel_check(result.isLeft() && result.left().value == 1);
    result = BooleanEncoder<EB>().encode(backend, false);
    spargel_check(result.isLeft() && result.left().value == 0);

    result = I32Encoder<EB>().encode(backend, -0x80000000);
    spargel_check(result.isLeft() && (i32)result.left().value == (i32)-0x80000000);
    result = I32Encoder<EB>().encode(backend, 0x7FFFFFFF);
    spargel_check(result.isLeft() && result.left().value == 0x7FFFFFFF);

    result = StringEncoder<EB>().encode(backend, base::string("ABC"));
    spargel_check(result.isLeft() && result.left().value == 3);
}

TEST(Codec2_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        auto result = StringEncoder<EB>().arrayOf().encode(backend, base::move(v));
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
        auto result = I32Encoder<EB>().arrayOf().arrayOf().encode(backend, base::move(v));
        spargel_check(result.isLeft() && (i32)result.left().value == (1 + -2) + (3 + -4));
    }
}

TEST(Codec2_Encode_Map) {
    {
        Student student;
        student.name = "Alice";
        student.age = 20;
        student.happy = true;
        base::vector<f32> scores;
        scores.push(98);
        scores.push(87.5f);
        scores.push(92);
        student.scores = base::move(scores);

        auto result = studentEncoder.encode(backend, base::move(student));
        spargel_check(result.isLeft());
    }
}

TEST(Codec2_Encode_Map_FailFast) {
    {
        int counter = 0;
        auto result = RecordEncoder<EB, int>::group(
                          BooleanEncoder<EB>().fieldOf("bool").forGetter<int>([&](int n) { counter++; return true; }),
                          I32Encoder<EB>().fieldOf("i32").forGetter<int>([&](int n) { counter++; return 0; }),
                          StringEncoder<EB>().fieldOf("string").forGetter<int>([&](int n) { counter++; return base::string("string"); }))
                          .encode(backend, 0);
        spargel_check(result.isLeft());
        spargel_check(counter == 3);
    }
    {
        int counter = 0;
        auto result = RecordEncoder<EB, int>::group(
                          BooleanEncoder<EB>().fieldOf("bool").forGetter<int>([&](int n) { counter++; return true; }),
                          ErrorEncoder<EB, i32>("error").fieldOf("i32").forGetter<int>([&](int n) { counter++; return 0; }),
                          StringEncoder<EB>().fieldOf("string").forGetter<int>([&](int n) { counter++; return base::string("string"); }))
                          .encode(backend, 0);
        spargel_check(result.isRight());
        spargel_check(counter == 2);
    }
}
