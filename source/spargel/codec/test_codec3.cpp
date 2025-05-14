#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/codec/codec3.h>

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

    using EB = TestEncodeBackend;
    using DB = TestDecodeBackend;
    using B = TestCodecBackend;

    auto encodeBackend = EB{};
    auto decodeBackend = DB{};

    // static type checks

    static_assert(Encoder3<NullEncoder<EB>>);
    static_assert(Decoder3<NullDecoder<DB>>);
    static_assert(Codec<NullCodec<B>>);

    static_assert(Encoder3<AsEncoder<NullCodec<B>>>);
    static_assert(Decoder3<AsDecoder<NullCodec<B>>>);

    static_assert(Encoder3<ErrorEncoder<EB, bool>>);
    static_assert(Decoder3<ErrorDecoder<DB, bool>>);
    static_assert(Codec<ErrorCodec<B, bool>>);

    static_assert(Encoder3<BooleanEncoder<EB>>);
    static_assert(Decoder3<BooleanDecoder<DB>>);
    static_assert(Codec<BooleanCodec<B>>);

    static_assert(Encoder3<I32Encoder<EB>>);
    static_assert(Decoder3<I32Decoder<DB>>);
    static_assert(Codec<I32Codec<B>>);
    static_assert(Encoder3<U32Encoder<EB>>);
    static_assert(Decoder3<U32Decoder<DB>>);
    static_assert(Codec<U32Codec<B>>);

    static_assert(Encoder3<StringEncoder<EB>>);
    static_assert(Decoder3<StringDecoder<DB>>);
    static_assert(Codec<StringCodec<B>>);

    static_assert(Encoder3<ArrayEncoder<NullEncoder<EB>>>);
    static_assert(Decoder3<ArrayDecoder<NullDecoder<DB>>>);
    static_assert(Codec<ArrayCodec<NullCodec<B>>>);

    static_assert(Encoder3<ArrayEncoder<ArrayEncoder<NullEncoder<EB>>>>);
    static_assert(Decoder3<ArrayDecoder<ArrayDecoder<NullDecoder<DB>>>>);
    static_assert(Codec<ArrayCodec<ArrayCodec<NullCodec<B>>>>);

}  // namespace

TEST(Codec_Encode_Error) {
    auto result = ErrorEncoder<EB, bool>("encode error"_sv).encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Codec_Decode_Error) {
    auto result = ErrorDecoder<DB, bool>("decode error"_sv).decode(decodeBackend, TestData{});
    spargel_check(result.isRight());
}

TEST(Codec_Encode_Primitive) {
    base::Either<TestData, CodecError> result = base::Left(TestData{});

    result = NullEncoder<EB>{}.encode(encodeBackend, nullptr);
    spargel_check(result.isLeft());

    result = BooleanEncoder<EB>{}.encode(encodeBackend, true);
    spargel_check(result.isLeft());

    result = U32Encoder<EB>{}.encode(encodeBackend, 0);
    spargel_check(result.isLeft());

    result = I32Encoder<EB>{}.encode(encodeBackend, 0);
    spargel_check(result.isLeft());

    result = StringEncoder<EB>{}.encode(encodeBackend, "ABC"_sv);
    spargel_check(result.isLeft());

    result = NullCodec<B>{}.encode(encodeBackend, nullptr);
    spargel_check(result.isLeft());
}

TEST(Codec_Decode_Primitive) {
    {
        auto result = NullDecoder<DB>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanDecoder<DB>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = U32Decoder<DB>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = I32Decoder<DB>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = StringDecoder<DB>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }

    {
        auto result = NullCodec<B>{}.decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        auto result = makeArrayEncoder(StringEncoder<EB>{}).encode(encodeBackend, base::move(v));
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
        auto result = makeArrayEncoder(makeArrayEncoder(I32Encoder<EB>{})).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }

    {
        base::vector<base::string> v;
        v.push("A");
        v.push("BC");
        v.push("DEF");
        auto result = makeArrayCodec(StringCodec<B>{}).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft());
    }
}

TEST(Codec_Decode_Array) {
    {
        auto result = makeArrayDecoder(I32Decoder<DB>{}).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
    {
        auto result = makeArrayDecoder(makeArrayDecoder(I32Decoder<DB>{})).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }

    {
        auto result = makeArrayCodec(I32Codec<B>{}).decode(decodeBackend, TestData{});
        spargel_check(result.isLeft());
    }
}
