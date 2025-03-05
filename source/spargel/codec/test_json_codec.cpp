#include <spargel/codec/test_json.h>

static_assert(EncodeBackend<EncodeBackedJson>);
static_assert(DecodeBackend<DecodeBackendJson>);

void test_encode_primitive() {
    EncodeBackedJson encoder;
    auto result = base::makeLeft<JsonValue, JsonEncodeError>();

    result = CodecBoolean::encode(encoder, true);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

    result = CodecBoolean::encode(encoder, false);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

    result = CodecString::encode(encoder, base::string("ABC"));
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonString("ABC")));

    result = CodecU32::encode(encoder, 123);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(123)));

    result = CodecI32::encode(encoder, -321);
    spargel_assert(result.isLeft() && isEqual(result.left(), JsonNumber(-321)));
}

void test_encode_array() {
    EncodeBackedJson encoder;

    {
        base::vector<base::string> v;
        v.push("ABC");
        v.push("123");
        v.push("!@#$");
        auto result = CodecArray<CodecString>::encode(encoder, base::move(v));
        spargel_assert(result.isLeft() && result.left().type == JsonValueType::array);
        auto& array = result.left().array.elements;
        spargel_assert(array.count() == 3);
        spargel_assert(isEqual(array[0], JsonString("ABC")));
        spargel_assert(isEqual(array[1], JsonString("123")));
        spargel_assert(isEqual(array[2], JsonString("!@#$")));
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
        auto result = CodecArray<CodecArray<CodecI32>>::encode(encoder, base::move(v));
        spargel_assert(result.isLeft() && result.left().type == JsonValueType::array);
        auto& array = result.left().array.elements;
        spargel_assert(array.count() == 2);
        spargel_assert(array[0].type == JsonValueType::array);
        spargel_assert(array[1].type == JsonValueType::array);
        auto& array1 = array[0].array.elements;
        spargel_assert(isEqual(array1[0], JsonNumber(1)));
        spargel_assert(isEqual(array1[1], JsonNumber(2)));
        auto& array2 = array[1].array.elements;
        spargel_assert(isEqual(array2[0], JsonNumber(-3)));
        spargel_assert(isEqual(array2[1], JsonNumber(-4)));
    }
}

void test_decode_primitive() {
    DecodeBackendJson decoder;

    {
        auto result = CodecBoolean::decode(decoder, JsonBoolean(true));
        spargel_assert(result.isLeft() && result.left() == true);
    }
    {
        auto result = CodecBoolean::decode(decoder, JsonBoolean(false));
        spargel_assert(result.isLeft() && result.left() == false);
    }
    {
        auto result = CodecString::decode(decoder, JsonString("ABC"));
        spargel_assert(result.isLeft() && result.left() == base::string("ABC"));
    }
    {
        auto result = CodecU32::decode(decoder, JsonNumber(123));
        spargel_assert(result.isLeft() && result.left() == 123);
    }
    {
        auto result = CodecU32::decode(decoder, JsonNumber(-123));
        spargel_assert(result.isRight());
    }
    {
        auto result = CodecU32::decode(decoder, JsonNumber(123.456));
        spargel_assert(result.isRight());
    }
    {
        auto result = CodecI32::decode(decoder, JsonNumber(-321));
        spargel_assert(result.isLeft() && result.left() == -321);
    }
}

void test_decode_array() {
    DecodeBackendJson decoder;

    {
        auto result_json = parseJson("[123, 456, 789]");
        spargel_assert(result_json.isLeft());
        auto result = CodecArray<CodecU32>::decode(decoder, base::move(result_json.left()));
        spargel_assert(result.isLeft());
        auto& array = result.left();
        spargel_assert(array.count() == 3);
        spargel_assert(array[0] == 123);
        spargel_assert(array[1] == 456);
        spargel_assert(array[2] == 789);
    }
    {
        auto result_json = parseJson("[[\"ABC\", \"123\"], [\"XYZ\", \"789\"]]");
        spargel_assert(result_json.isLeft());
        auto result =
            CodecArray<CodecArray<CodecString>>::decode(decoder, base::move(result_json.left()));
        spargel_assert(result.isLeft());
        auto& array = result.left();
        spargel_assert(array[0][0] == base::string("ABC"));
        spargel_assert(array[0][1] == base::string("123"));
        spargel_assert(array[1][0] == base::string("XYZ"));
        spargel_assert(array[1][1] == base::string("789"));
    }
}

int main() {
    test_encode_primitive();

    test_encode_array();

    test_decode_primitive();

    test_decode_array();

    return 0;
}
