#include <spargel/codec/test_json.h>

void test_encode_primitive() {
    EncodeBackedJson encoder;
    auto result = base::makeLeft<JsonValue, JsonEncodeError>();

    result = CodecBoolean::encode(true, encoder);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

    result = CodecBoolean::encode(false, encoder);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

    result = CodecString::encode(base::string("ABC"), encoder);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("ABC")));

    result = CodecU32::encode(123, encoder);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(123)));

    result = CodecI32::encode(-321, encoder);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-321)));
}

void test_encode_array() {
    EncodeBackedJson encoder;

    base::vector<base::string> array;
    array.push("ABC");
    array.push("123");
    array.push("!@#$");
    auto result = CodecArray<CodecString>::encode(base::move(array), encoder);
    spargel_assert(result.isLeft() && result.left().type == JsonValueType::array);
    auto array_json = result.left().array.elements;
    spargel_assert(array_json.count() == 3);
    spargel_assert(isEqual(array_json[0], JsonString("ABC")));
    spargel_assert(isEqual(array_json[1], JsonString("123")));
    spargel_assert(isEqual(array_json[2], JsonString("!@#$")));
}

void test_decode_primitive() {
    DecodeBackendJson decoder;

    auto result1 = CodecBoolean::decode(JsonBoolean(true), decoder);
    spargel_assert(result1.isLeft() && result1.left() == true);

    auto result2 = CodecBoolean::decode(JsonBoolean(false), decoder);
    spargel_assert(result2.isLeft() && result2.left() == false);

    auto result3 = CodecString::decode(JsonString("ABC"), decoder);
    spargel_assert(result3.isLeft() && result3.left() == base::string("ABC"));

    auto result4 = CodecU32::decode(JsonNumber(123), decoder);
    spargel_assert(result4.isLeft() && result4.left() == 123);

    auto result5 = CodecI32::decode(JsonNumber(-321), decoder);
    spargel_assert(result5.isLeft() && result5.left() == -321);
}

void test_decode_array() {
    DecodeBackendJson decoder;
    base::Either<JsonValue, JsonParseError> result_json =
        base::makeLeft<JsonValue, JsonParseError>();

    result_json = parseJson("[123, 456, 789]");
    spargel_assert(result_json.isLeft());
    auto result1 = CodecArray<CodecU32>::decode(base::move(result_json.left()), decoder);
    spargel_assert(result1.isLeft());
    auto array1 = result1.left();
    spargel_assert(array1.count() == 3);
    spargel_assert(array1[0] == 123);
    spargel_assert(array1[1] == 456);
    spargel_assert(array1[2] == 789);

    result_json = parseJson("[[1, 2], [3, 4]]");
    spargel_assert(result_json.isLeft());
    auto result2 =
        CodecArray<CodecArray<CodecU32>>::decode(base::move(result_json.left()), decoder);
    spargel_assert(result2.isLeft());
    auto array2 = result2.left();
    spargel_assert(array2[0][0] == 1);
    spargel_assert(array2[0][1] == 2);
    spargel_assert(array2[1][0] == 3);
    spargel_assert(array2[1][1] == 4);
}

int main() {
    test_encode_primitive();

    test_encode_array();

    test_decode_primitive();

    test_decode_array();

    return 0;
}
