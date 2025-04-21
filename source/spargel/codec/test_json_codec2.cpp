#include <spargel/codec/codec2.h>
#include <spargel/codec/test_json.h>

#include "json.h"

namespace {

    struct Student {
        base::string type = base::string("normal");
        base::string name;
        base::Optional<base::string> nickname;
        u32 age;
        bool happy;
        base::vector<f32> scores;

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
    };

    using EB = JsonEncodeBackend;
    using DB = JsonDecodeBackend;

    auto encodeBackend = EB();
    auto decodeBackend = DB();

    auto studentEncoder = Student::encoder<EB>();

}  // namespace

TEST(Json_Codec_Encode_Error) {
    auto result = ErrorEncoder<EB, bool>("encode error").encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Json_Codec_Decode_Error) {
    auto result = ErrorDecoder<DB, bool>("decode error").decode(decodeBackend, JsonValue());
    spargel_check(result.isRight());
}

TEST(Json_Codec_Encode_Primitive) {
    base::Either<JsonValue, JsonParseError> result = base::Left(JsonValue());

    result = NullEncoder<EB>().encode(encodeBackend, nullptr);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNull()));

    result = BooleanEncoder<EB>().encode(encodeBackend, true);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

    result = BooleanEncoder<EB>().encode(encodeBackend, false);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

    result = U32Encoder<EB>().encode(encodeBackend, 4294967295);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(4294967295)));

    result = I32Encoder<EB>().encode(encodeBackend, -2147483648);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(-2147483648)));

    result = F32Encoder<EB>().encode(encodeBackend, 123.456f);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(123.456f)));

    result = StringEncoder<EB>().encode(encodeBackend, base::string("ABC"));
    spargel_check(result.isLeft() && isEqual(result.left(), JsonString("ABC")));
}

TEST(Json_Codec_Decode_Primitive) {
    {
        auto result = NullDecoder<DB>().decode(decodeBackend, JsonValue(JsonNull()));
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanDecoder<DB>().decode(decodeBackend, JsonValue(JsonBoolean(true)));
        spargel_check(result.isLeft() && result.left() == true);
    }
    {
        auto result = BooleanDecoder<DB>().decode(decodeBackend, JsonValue(JsonBoolean(false)));
        spargel_check(result.isLeft() && result.left() == false);
    }
    {
        auto result = U32Decoder<DB>().decode(decodeBackend, JsonValue(JsonNumber(4294967295)));
        spargel_check(result.isLeft() && result.left() == 4294967295);
    }
    {
        auto result = I32Decoder<DB>().decode(decodeBackend, JsonValue(JsonNumber(-2147483648)));
        spargel_check(result.isLeft() && result.left() == -2147483648);
    }
    {
        auto result = F32Decoder<DB>().decode(decodeBackend, JsonValue(JsonNumber(123.456f)));
        spargel_check(result.isLeft() && fabs(result.left() - 123.456f) < 1e-6f);
    }
    {
        auto result = StringDecoder<DB>().decode(decodeBackend, JsonValue(JsonString("ABC")));
        spargel_check(result.isLeft() && result.left() == base::string("ABC"));
    }
}

TEST(Json_Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("ABC");
        v.push("123");
        v.push("!@#$");
        auto result = StringEncoder<EB>().arrayOf().encode(encodeBackend, base::move(v));
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
        auto result = I32Encoder<EB>().arrayOf().arrayOf().encode(encodeBackend, base::move(v));
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

TEST(Json_Codec_Decode_array) {
    {
        auto result_json = parseJson("[123, 456, 789]");
        spargel_check(result_json.isLeft());

        auto result = U32Decoder<DB>().arrayOf().decode(decodeBackend, base::move(result_json.left()));
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

        auto result = StringDecoder<DB>().arrayOf().arrayOf().decode(decodeBackend, base::move(result_json.left()));
        spargel_check(result.isLeft());

        auto& array = result.left();
        spargel_check(array[0][0] == base::string("ABC"));
        spargel_check(array[0][1] == base::string("123"));
        spargel_check(array[1][0] == base::string("XYZ"));
        spargel_check(array[1][1] == base::string("789"));
    }
}

TEST(Json_Codec_Encode_Record) {
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

        auto& object = result.left().object;
        spargel_check(isMemberEqual(object, JsonString("type"), JsonString("normal")));
        spargel_check(isMemberEqual(object, JsonString("name"), JsonString("Alice")));
        auto* p_nickname = object.members.get(JsonString("nickname"));
        spargel_check(p_nickname == nullptr);
        spargel_check(isMemberEqual(object, JsonString("age"), JsonNumber(20)));
        spargel_check(isMemberEqual(object, JsonString("happy"), JsonBoolean(true)));
        auto* p_array = object.members.get(JsonString("scores"));
        spargel_check(p_array != nullptr && p_array->type == JsonValueType::array);
        auto& array = p_array->array.elements;
        spargel_check(array.count() == 3);
        spargel_check(isEqual(array[0], JsonNumber(98)));
        spargel_check(isEqual(array[1], JsonNumber(87.5)));
        spargel_check(isEqual(array[2], JsonNumber(92)));
    }
}

TEST(Json_Codec_Decode_Record) {
    {
        const auto str =
            "{\n"
            "  \"name\": \"Alice\",\n"
            "  \"age\": 20,\n"
            "  \"happy\": true,\n"
            "  \"scores\": [98, 87.5, 92]\n"
            "}";
        auto result_json = parseJson(str);
        spargel_check(result_json.isLeft());
    }
}
