#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/codec/codec2.h>

// libc
#include <math.h>

#include "codec.h"

using namespace spargel;
using namespace spargel::codec;

namespace {

    struct TestData {};

    struct EncodeBackendTest {
        using DataType = TestData;
        using ErrorType = CodecError;

        base::Either<TestData, CodecError> makeNull() { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeBoolean(bool b) { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeU8(u8 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeI8(i8 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeU16(u16 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeI16(i16 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeU32(u32 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeI32(i32 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeU64(u64 n) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeI64(i64 n) { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeF32(f32 v) { return base::Left(TestData{}); }
        base::Either<TestData, CodecError> makeF64(f64 v) { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeString(const base::string& s) { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeArray(const base::vector<TestData>& array) { return base::Left(TestData{}); }

        base::Either<TestData, CodecError> makeMap(const base::HashMap<base::string, TestData>& map) { return base::Left(TestData{}); }
    };

    static_assert(EncodeBackend<EncodeBackendTest>);

    struct DecodeBackendTest {
        using DataType = TestData;
        using ErrorType = CodecError;

        base::Optional<CodecError> getNull(const TestData& data) { return base::nullopt; }

        base::Either<bool, CodecError> getBoolean(const TestData& data) { return base::Left(true); }

        base::Either<u8, CodecError> getU8(const TestData& data) { return base::Left((u8)0); }
        base::Either<i8, CodecError> getI8(const TestData& data) { return base::Left((i8)0); }
        base::Either<u16, CodecError> getU16(const TestData& data) { return base::Left((u16)0); }
        base::Either<i16, CodecError> getI16(const TestData& data) { return base::Left((i16)0); }
        base::Either<u32, CodecError> getU32(const TestData& data) { return base::Left((u32)0); }
        base::Either<i32, CodecError> getI32(const TestData& data) { return base::Left((i32)0); }
        base::Either<u64, CodecError> getU64(const TestData& data) { return base::Left((u64)0); }
        base::Either<i64, CodecError> getI64(const TestData& data) { return base::Left((i64)0); }

        base::Either<f32, CodecError> getF32(const TestData& data) { return base::Left((f32)0); }
        base::Either<f64, CodecError> getF64(const TestData& data) { return base::Left((f64)0); }

        base::Either<base::string, CodecError> getString(const TestData& data) { return base::Left(base::string("")); }

        base::Either<base::vector<TestData>, CodecError> getArray(const TestData& data) {
            return base::Left(base::vector<TestData>());
        }

        base::Either<base::Optional<TestData>, CodecError> getMember(const TestData& data, base::string_view key) {
            return base::Left(base::makeOptional<TestData>());
        }
    };

    static_assert(DecodeBackend<DecodeBackendTest>);

    struct Student {
        base::string type = base::string("normal");
        base::string name;
        base::Optional<base::string> nickname;
        u32 age;
        bool happy;
        base::vector<f32> scores;

        using Constructor = base::Constructor<Student(const base::string&, const base::string&,
                                                      const base::Optional<base::string>&, u32, bool,
                                                      const base::vector<f32>&)>;

        template <EncodeBackend EB>
        static auto encoder() {
            return makeRecordEncoder<EB, Student>(
                StringEncoder<EB>().fieldOf("type").template forGetter<Student>([](const Student& student) { return student.type; }),
                StringEncoder<EB>().fieldOf("name").template forGetter<Student>([](const Student& student) { return student.name; }),
                StringEncoder<EB>().optionalFieldOf("nickname").template forGetter<Student>([](const Student& student) { return student.nickname; }),
                U32Encoder<EB>().fieldOf("age").template forGetter<Student>([](const Student& student) { return student.age; }),
                BooleanEncoder<EB>().fieldOf("happy").template forGetter<Student>([](const Student& student) { return student.happy; }),
                F32Encoder<EB>().arrayOf().fieldOf("scores").template forGetter<Student>([](const Student& student) { return student.scores; }));
        }

        template <DecodeBackend DB>
        static auto decoder() {
            return makeRecordDecoder<DB, Student>(
                Constructor(),
                StringDecoder<DB>().fieldOf("type"),
                StringDecoder<DB>().fieldOf("name"),
                StringDecoder<DB>().optionalFieldOf("nickname"),
                U32Decoder<DB>().fieldOf("age"),
                BooleanDecoder<DB>().fieldOf("happy"),
                F32Decoder<DB>().arrayOf().fieldOf("scores"));
        }
    };

    using EB = EncodeBackendTest;
    using DB = DecodeBackendTest;

    auto encodeBackend = EB{};
    auto decodeBackend = DB{};

    auto studentEncoder = Student::encoder<EB>();
    auto studentDecoder = Student::decoder<DB>();

}  // namespace

TEST(Codec_Encode_Error) {
    auto result = ErrorEncoder<EB, bool>("error").encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Codec_Decode_Error) {
    auto result = ErrorDecoder<DB, bool>("decode error").decode(decodeBackend, TestData{});
    spargel_check(result.isRight());
}

TEST(Codec_Encode_Primitive) {
    auto result = base::makeRight<TestData, CodecError>("");

    result = NullEncoder<EB>().encode(encodeBackend, nullptr);
    spargel_check(result.isLeft());

    result = BooleanEncoder<EB>().encode(encodeBackend, true);
    spargel_check(result.isLeft());

    result = I32Encoder<EB>().encode(encodeBackend, 0);
    spargel_check(result.isLeft());

    result = StringEncoder<EB>().encode(encodeBackend, base::string("ABC"));
    spargel_check(result.isLeft());
}

TEST(Codec_Decode_Primitive) {
    {
        auto result = NullDecoder<DB>().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanDecoder<DB>().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = I32Decoder<DB>().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = StringDecoder<DB>().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        auto result = StringEncoder<EB>().arrayOf().encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
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
        auto result = I32Encoder<EB>().arrayOf().arrayOf().encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Decode_Array) {
    {
        auto result = I32Decoder<DB>().arrayOf().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = StringDecoder<DB>().arrayOf().arrayOf().decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Map) {
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

        auto result = studentEncoder.encode(encodeBackend, base::move(student));
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Decode_Map) {
    {
        auto result = studentDecoder.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Map_FailFast) {
    {
        int counter = 0;
        auto result = makeRecordEncoder<EB, int>(
                          BooleanEncoder<EB>().fieldOf("bool").forGetter<int>([&](int n) { counter++; return true; }),
                          I32Encoder<EB>().fieldOf("i32").forGetter<int>([&](int n) { counter++; return 0; }),
                          StringEncoder<EB>().fieldOf("string").forGetter<int>([&](int n) { counter++; return base::string("string"); }))
                          .encode(encodeBackend, 0);
        spargel_check(result.isLeft());
        spargel_check(counter == 3);
    }
    {
        int counter = 0;
        auto result = makeRecordEncoder<EB, int>(
                          BooleanEncoder<EB>().fieldOf("bool").forGetter<int>([&](int n) { counter++; return true; }),
                          ErrorEncoder<EB, i32>("error").fieldOf("i32").forGetter<int>([&](int n) { counter++; return 0; }),
                          StringEncoder<EB>().fieldOf("string").forGetter<int>([&](int n) { counter++; return base::string("string"); }))
                          .encode(encodeBackend, 0);
        spargel_check(result.isRight());
        spargel_check(counter == 2);
    }
}
