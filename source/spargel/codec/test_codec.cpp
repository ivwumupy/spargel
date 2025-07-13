#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/codec/codec.h>

using namespace spargel;
using namespace spargel::codec;
using namespace spargel::base::literals;

namespace {

    struct TestData {};

    struct TestEncodeBackend {
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

    static_assert(EncodeBackend<TestEncodeBackend>);

    struct TestDecodeBackend {
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
            return base::Left(base::vector<TestData>{});
        }

        base::Either<base::Optional<TestData>, CodecError> getMember(const TestData& data, base::string_view key) {
            return base::Left(base::makeOptional<TestData>());
        }
    };

    static_assert(DecodeBackend<TestDecodeBackend>);

    struct TestCodecBackend {
        using EncodeBackendType = TestEncodeBackend;
        using DecodeBackendType = TestDecodeBackend;
    };

    static_assert(CodecBackend<TestCodecBackend>);

    struct Student {
        base::string type = base::string("normal");
        base::string name;
        base::Optional<base::string> nickname;
        u32 age;
        bool happy;
        base::vector<f32> scores;

        static auto encoder() {
            return makeRecordEncoder<Student>(
                makeNormalEncodeField<Student>("type"_sv, StringCodec{}, [](auto& o) { return o.type; }),
                makeNormalEncodeField<Student>("name"_sv, StringCodec{}, [](auto& o) { return o.name; }),
                makeOptionalEncodeField<Student>("nickname"_sv, StringCodec{}, [](auto& o) { return o.nickname; }),
                makeNormalEncodeField<Student>("age"_sv, U32Codec{}, [](auto& o) { return o.age; }),
                makeNormalEncodeField<Student>("happy"_sv, BooleanCodec{}, [](auto& o) { return o.happy; }),
                makeNormalEncodeField<Student>("scores"_sv, makeVectorEncoder(F32Codec{}), [](auto& o) { return o.scores; }));
        }

        static auto decoder() {
            return makeRecordDecoder<Student>(
                base::Constructor<Student>{},
                makeDefaultDecodeField("type"_sv, StringCodec{}, base::string("normal")),
                makeNormalDecodeField("name"_sv, StringCodec{}),
                makeOptionalDecodeField("nickname"_sv, StringCodec{}),
                makeNormalDecodeField("age"_sv, U32Codec{}),
                makeNormalDecodeField("happy"_sv, BooleanCodec{}),
                makeNormalDecodeField("scores"_sv, makeVectorDecoder(F32Codec{})));
        }

        static auto codec() {
            return makeRecordCodec<Student>(
                base::Constructor<Student>{},
                makeDefaultField<Student>("type"_sv, StringCodec{}, base::string("normal"), [](auto& o) { return o.type; }),
                makeNormalField<Student>("name"_sv, StringCodec{}, [](auto& o) { return o.name; }),
                makeOptionalField<Student>("nickname"_sv, StringCodec{}, [](auto& o) { return o.nickname; }),
                makeNormalField<Student>("age"_sv, U32Codec{}, [](auto& o) { return o.age; }),
                makeNormalField<Student>("happy"_sv, BooleanCodec{}, [](auto& o) { return o.happy; }),
                makeNormalField<Student>("scores"_sv, makeVectorCodec(F32Codec{}), [](auto& o) { return o.scores; }));
        }
    };

    using EB = TestEncodeBackend;
    using DB = TestDecodeBackend;
    using B = TestCodecBackend;

    auto encodeBackend = EB{};
    auto decodeBackend = DB{};

    auto studentEncoder = Student::encoder();
    auto studentDecoder = Student::decoder();
    auto studentCodec = Student::codec();

    // static type checks

    static_assert(Encoder<ErrorEncoder<bool>>);
    static_assert(Decoder<ErrorDecoder<bool>>);
    static_assert(Codec<ErrorCodec<bool>>);

    static_assert(Codec<NullCodec>);

    static_assert(Codec<BooleanCodec>);

    static_assert(Codec<I32Codec>);
    static_assert(Codec<U32Codec>);

    static_assert(Codec<F32Codec>);
    static_assert(Codec<F64Codec>);

    static_assert(Codec<StringCodec>);

    static_assert(Encoder<VectorEncoder<NullCodec>>);
    static_assert(Decoder<VectorDecoder<NullCodec>>);
    static_assert(Codec<VectorCodec<NullCodec>>);

    static_assert(Encoder<VectorEncoder<VectorEncoder<NullCodec>>>);
    static_assert(Decoder<VectorDecoder<VectorDecoder<NullCodec>>>);
    static_assert(Codec<VectorCodec<VectorCodec<NullCodec>>>);

    static_assert(FieldEncoder<NormalFieldEncoder<NullCodec>>);
    static_assert(FieldDecoder<NormalFieldDecoder<NullCodec>>);
    static_assert(FieldCodec<NormalFieldCodec<NullCodec>>);

    static_assert(FieldEncoder<OptionalFieldEncoder<NullCodec>>);
    static_assert(FieldDecoder<OptionalFieldDecoder<NullCodec>>);
    static_assert(FieldCodec<OptionalFieldCodec<NullCodec>>);

    static_assert(Encoder<decltype(Student::encoder())>);
    static_assert(Decoder<decltype(Student::decoder())>);
    static_assert(Codec<decltype(Student::codec())>);

}  // namespace

TEST(Codec_Encode_Error) {
    auto result = ErrorEncoder<bool>("encode error"_sv).encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Codec_Decode_Error) {
    auto result = ErrorDecoder<bool>("decode error"_sv).decode(decodeBackend, TestData{});
    spargel_check(result.isRight());
}

TEST(Codec_Encode_Primitive) {
    base::Either<TestData, CodecError> result = base::Left(TestData{});

    result = NullCodec{}.encode(encodeBackend, nullptr);
    spargel_check(result.isLeft());

    result = BooleanCodec{}.encode(encodeBackend, true);
    spargel_check(result.isLeft());

    result = U32Codec{}.encode(encodeBackend, 0);
    spargel_check(result.isLeft());

    result = I32Codec{}.encode(encodeBackend, 0);
    spargel_check(result.isLeft());

    result = F32Codec{}.encode(encodeBackend, 3.14f);
    spargel_check(result.isLeft());

    result = F64Codec{}.encode(encodeBackend, 12345.6789);
    spargel_check(result.isLeft());

    result = StringCodec{}.encode(encodeBackend, "ABC"_sv);
    spargel_check(result.isLeft());
}

TEST(Codec_Decode_Primitive) {
    {
        auto result = NullCodec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanCodec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = U32Codec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = I32Codec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = F32Codec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = F64Codec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = StringCodec{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.emplace("A");
        v.emplace("BC");
        v.emplace("DEF");
        auto result = makeVectorEncoder(StringCodec{}).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }
    {
        base::vector<base::vector<i32>> v;
        base::vector<i32> v1;
        v1.emplace(1);
        v1.emplace(-2);
        v.emplace(base::move(v1));
        base::vector<i32> v2;
        v2.emplace(3);
        v2.emplace(-4);
        v.emplace(base::move(v2));
        auto result = makeVectorEncoder(makeVectorEncoder(I32Codec{})).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }

    {
        base::vector<base::string> v;
        v.emplace("A");
        v.emplace("BC");
        v.emplace("DEF");
        auto result = makeVectorCodec(StringCodec{}).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Decode_Array) {
    {
        auto result = makeVectorDecoder(I32Codec{}).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = makeVectorDecoder(makeVectorDecoder(I32Codec{})).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }

    {
        auto result = makeVectorCodec(I32Codec{}).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Record) {
    Student student;
    student.name = "Alice";
    student.age = 20;
    student.happy = true;
    base::vector<f32> scores;
    scores.emplace(98);
    scores.emplace(87.5f);
    scores.emplace(92);
    student.scores = base::move(scores);

    {
        auto result = studentEncoder.encode(encodeBackend, student);
        spargel_check(result.isLeft());
    }
    {
        auto result = studentCodec.encode(encodeBackend, student);
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Decode_Record) {
    {
        auto result = studentDecoder.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = studentCodec.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Record_FailFast) {
    {
        int counter = 0;
        auto result = makeRecordEncoder<int>(
                          makeNormalEncodeField<int>("boo"_sv, BooleanCodec{}, [&](int n) {counter++; return true; }),
                          makeNormalEncodeField<int>("i32"_sv, I32Codec{}, [&](int n) {counter++; return 0; }),
                          makeNormalEncodeField<int>("string"_sv, StringCodec{}, [&](int n) {counter++; return "string"_sv; }))
                          .encode(encodeBackend, 0);
        spargel_check(result.isLeft());
        spargel_check(counter == 3);
    }
    {
        int counter = 0;
        auto result = makeRecordEncoder<int>(
                          makeNormalEncodeField<int>("boo"_sv, BooleanCodec{}, [&](int n) {counter++; return true; }),
                          makeNormalEncodeField<int>("i32"_sv, ErrorEncoder<i32>("error"_sv), [&](int n) {counter++; return 0; }),
                          makeNormalEncodeField<int>("string"_sv, StringCodec{}, [&](int n) {counter++; return "string"_sv; }))
                          .encode(encodeBackend, 0);
        spargel_check(result.isRight());
        spargel_check(counter == 2);
    }
}
