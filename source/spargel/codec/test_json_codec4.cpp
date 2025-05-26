#include <spargel/base/string_view.h>
#include <spargel/codec/codec4.h>
#include <spargel/codec/test_json.h>

using namespace spargel::base::literals;

namespace {

    static_assert(CodecBackend<JsonCodecBackend>);

    auto encodeBackend = JsonEncodeBackend();
    auto decodeBackend = JsonDecodeBackend();

}  // namespace

TEST(Json_Codec_Encode_Error) {
    auto result = ErrorCodec<bool>("encode error"_sv, "decode_error"_sv).encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Json_Codec_Decode_Error) {
    auto result = ErrorCodec<bool>("encode error"_sv, "decode_error"_sv).decode(decodeBackend, JsonValue());
    spargel_check(result.isRight());
}

TEST(Json_Codec_Encode_Primitive) {
    base::Either<JsonValue, JsonParseError> result = base::Left(JsonValue());

    result = NullCodec{}.encode(encodeBackend, nullptr);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNull()));

    result = BooleanCodec{}.encode(encodeBackend, true);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

    result = BooleanCodec{}.encode(encodeBackend, false);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

    result = U32Codec{}.encode(encodeBackend, 4294967295);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(4294967295)));

    result = I32Codec{}.encode(encodeBackend, -2147483648);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(-2147483648)));

    result = F32Codec{}.encode(encodeBackend, 123.456f);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(123.456f)));

    result = F64Codec{}.encode(encodeBackend, 789.012);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(789.012)));

    result = StringCodec{}.encode(encodeBackend, base::string("ABC"));
    spargel_check(result.isLeft() && isEqual(result.left(), JsonString("ABC")));
}

TEST(Json_Codec_Decode_Primitive) {
    {
        auto result = NullCodec{}.decode(decodeBackend, JsonValue(JsonNull()));
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanCodec{}.decode(decodeBackend, JsonValue(JsonBoolean(true)));
        spargel_check(result.isLeft() && result.left() == true);
    }
    {
        auto result = BooleanCodec{}.decode(decodeBackend, JsonValue(JsonBoolean(false)));
        spargel_check(result.isLeft() && result.left() == false);
    }
    {
        auto result = U32Codec{}.decode(decodeBackend, JsonValue(JsonNumber(4294967295)));
        spargel_check(result.isLeft() && result.left() == 4294967295);
    }
    {
        auto result = I32Codec{}.decode(decodeBackend, JsonValue(JsonNumber(-2147483648)));
        spargel_check(result.isLeft() && result.left() == -2147483648);
    }
    {
        auto result = F32Codec{}.decode(decodeBackend, JsonValue(JsonNumber(123.456f)));
        spargel_check(result.isLeft() && fabs(result.left() - 123.456f) < 1e-6f);
    }
    {
        auto result = F64Codec{}.decode(decodeBackend, JsonValue(JsonNumber(789.012)));
        spargel_check(result.isLeft() && fabs(result.left() - 789.012) < 1e-6f);
    }
    {
        auto result = StringCodec{}.decode(decodeBackend, JsonValue(JsonString("ABC")));
        spargel_check(result.isLeft() && result.left() == base::string("ABC"));
    }
}

TEST(Json_Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("ABC");
        v.push("123");
        v.push("!@#$");
        auto result = makeVectorCodec(StringCodec{}).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft() && result.left().type == JsonValueType::array);

        auto& array = result.left().array.elements;
        spargel_check(array.count() == 3);
        spargel_check(isEqual(array[0], JsonString("ABC")));
        spargel_check(isEqual(array[1], JsonString("123")));
        spargel_check(isEqual(array[2], JsonString("!@#$")));
    }
    {
        base::vector<base::vector<i32>> v;
        base::vector<i32> v1;
        v1.push(1);
        v1.push(2);
        v.push(base::move(v1));
        base::vector<i32> v2;
        v2.push(-3);
        v2.push(-4);
        v.push(base::move(v2));
        auto result = makeVectorCodec(makeVectorCodec(I32Codec{})).encode(encodeBackend, base::move(v));
        spargel_check(result.isLeft() && result.left().type == JsonValueType::array);

        auto& array = result.left().array.elements;
        spargel_check(array.count() == 2);
        spargel_check(array[0].type == JsonValueType::array);
        spargel_check(array[1].type == JsonValueType::array);
        auto& array1 = array[0].array.elements;
        spargel_check(isEqual(array1[0], JsonNumber(1)));
        spargel_check(isEqual(array1[1], JsonNumber(2)));
        auto& array2 = array[1].array.elements;
        spargel_check(isEqual(array2[0], JsonNumber(-3)));
        spargel_check(isEqual(array2[1], JsonNumber(-4)));
    }
}

TEST(Json_Codec_Decode_Array) {
    {
        auto result_json = parseJson("[123, 456, 789]");
        spargel_check(result_json.isLeft());

        auto result = makeVectorCodec(U32Codec{}).decode(decodeBackend, base::move(result_json.left()));
        spargel_check(result.isLeft());

        auto& array = result.left();
        spargel_check(array.count() == 3);
        spargel_check(array[0] == 123);
        spargel_check(array[1] == 456);
        spargel_check(array[2] == 789);
    }
    {
        auto result_json = parseJson("[[\"ABC\", \"123\"], [\"XYZ\", \"789\"]]");
        spargel_check(result_json.isLeft());

        auto result = makeVectorCodec(makeVectorCodec(StringCodec{})).decode(decodeBackend, base::move(result_json.left()));
        spargel_check(result.isLeft());

        auto& array = result.left();
        spargel_check(array[0][0] == base::string("ABC"));
        spargel_check(array[0][1] == base::string("123"));
        spargel_check(array[1][0] == base::string("XYZ"));
        spargel_check(array[1][1] == base::string("789"));
    }
}
