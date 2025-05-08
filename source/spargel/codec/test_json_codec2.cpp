#include <spargel/base/string_view.h>
#include <spargel/codec/codec2.h>
#include <spargel/codec/test_json.h>

using namespace spargel::base::literals;

namespace {

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

        template <CodecBackend B>
        static auto codec() {
            return makeRecordCodec<B, Student>(
                Constructor(),
                StringCodec<B>().defaultFieldOf("type"_sv, base::string("normal")).template forGetter<Student>([](const Student& student) { return student.type; }),
                StringCodec<B>().fieldOf("name"_sv).template forGetter<Student>([](const Student& student) { return student.name; }),
                StringCodec<B>().optionalFieldOf("nickname"_sv).template forGetter<Student>([](const Student& student) { return student.nickname; }),
                U32Codec<B>().fieldOf("age"_sv).template forGetter<Student>([](const Student& student) { return student.age; }),
                BooleanCodec<B>().fieldOf("happy"_sv).template forGetter<Student>([](const Student& student) { return student.happy; }),
                F32Codec<B>().arrayOf().fieldOf("scores"_sv).template forGetter<Student>([](const Student& student) { return student.scores; }));
        }
    };

    static_assert(CodecBackend<JsonCodecBackend>);

    auto encodeBackend = JsonEncodeBackend();
    auto decodeBackend = JsonDecodeBackend();

    auto studentCodec = Student::codec<JsonCodecBackend>();

    using B = JsonCodecBackend;

}  // namespace

TEST(Json_Codec_Encode_Error) {
    auto result = ErrorCodec<B, bool>("encode error"_sv, "decode_error"_sv).encode(encodeBackend, true);
    spargel_check(result.isRight());
}

TEST(Json_Codec_Decode_Error) {
    auto result = ErrorCodec<B, bool>("encode error"_sv, "decode_error"_sv).decode(decodeBackend, JsonValue());
    spargel_check(result.isRight());
}

TEST(Json_Codec_Encode_Primitive) {
    base::Either<JsonValue, JsonParseError> result = base::Left(JsonValue());

    result = NullCodec<B>().encode(encodeBackend, nullptr);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNull()));

    result = BooleanCodec<B>().encode(encodeBackend, true);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(true)));

    result = BooleanCodec<B>().encode(encodeBackend, false);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonBoolean(false)));

    result = U32Codec<B>().encode(encodeBackend, 4294967295);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(4294967295)));

    result = I32Codec<B>().encode(encodeBackend, -2147483648);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(-2147483648)));

    result = F32Codec<B>().encode(encodeBackend, 123.456f);
    spargel_check(result.isLeft() && isEqual(result.left(), JsonNumber(123.456f)));

    result = StringCodec<B>().encode(encodeBackend, base::string("ABC"));
    spargel_check(result.isLeft() && isEqual(result.left(), JsonString("ABC")));
}

TEST(Json_Codec_Decode_Primitive) {
    {
        auto result = NullCodec<B>().decode(decodeBackend, JsonValue(JsonNull()));
        spargel_check(result.isLeft());
    }
    {
        auto result = BooleanCodec<B>().decode(decodeBackend, JsonValue(JsonBoolean(true)));
        spargel_check(result.isLeft() && result.left() == true);
    }
    {
        auto result = BooleanCodec<B>().decode(decodeBackend, JsonValue(JsonBoolean(false)));
        spargel_check(result.isLeft() && result.left() == false);
    }
    {
        auto result = U32Codec<B>().decode(decodeBackend, JsonValue(JsonNumber(4294967295)));
        spargel_check(result.isLeft() && result.left() == 4294967295);
    }
    {
        auto result = I32Codec<B>().decode(decodeBackend, JsonValue(JsonNumber(-2147483648)));
        spargel_check(result.isLeft() && result.left() == -2147483648);
    }
    {
        auto result = F32Codec<B>().decode(decodeBackend, JsonValue(JsonNumber(123.456f)));
        spargel_check(result.isLeft() && fabs(result.left() - 123.456f) < 1e-6f);
    }
    {
        auto result = StringCodec<B>().decode(decodeBackend, JsonValue(JsonString("ABC")));
        spargel_check(result.isLeft() && result.left() == base::string("ABC"));
    }
}

TEST(Json_Codec_Encode_Array) {
    {
        base::vector<base::string> v;
        v.push("ABC");
        v.push("123");
        v.push("!@#$");
        auto result = StringCodec<B>().arrayOf().encode(encodeBackend, base::move(v));
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
        auto result = I32Codec<B>().arrayOf().arrayOf().encode(encodeBackend, base::move(v));
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

        auto result = U32Codec<B>().arrayOf().decode(decodeBackend, base::move(result_json.left()));
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

        auto result = StringCodec<B>().arrayOf().arrayOf().decode(decodeBackend, base::move(result_json.left()));
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

        auto result = studentCodec.encode(encodeBackend, base::move(student));
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
            "  \"type\": \"normal\",\n"
            "  \"name\": \"Alice\",\n"
            "  \"age\": 20,\n"
            "  \"happy\": true,\n"
            "  \"scores\": [98, 87.5, 92]\n"
            "}";
        auto result_json = parseJson(str);
        spargel_check(result_json.isLeft());

        auto result = studentCodec.decode(decodeBackend, base::move(result_json.left()));
        spargel_check(result.isLeft());

        auto& student = result.left();
        spargel_check(student.type == base::string("normal"));
        spargel_check(student.name == base::string("Alice"));
        spargel_check(!student.nickname.hasValue());
        spargel_check(student.age == 20);
        spargel_check(student.happy == true);
        spargel_check(student.scores.count() == 3);
        spargel_check(fabs(student.scores[0] - 98) < 1e-6f);
        spargel_check(fabs(student.scores[1] - 87.5f) < 1e-6f);
        spargel_check(fabs(student.scores[2] - 92) < 1e-6f);
    }
    {
        const auto str =
            "[\n"
            "  {\n"
            "    \"type\": \"normal\",\n"
            "    \"name\": \"Alice\",\n"
            "    \"age\": 20,\n"
            "    \"happy\": true,\n"
            "    \"scores\": [98, 87.5, 92]\n"
            "  },\n"
            "  {\n"
            "    \"name\": \"Bob\",\n"
            "    \"nickname\": \"Bomb\",\n"
            "    \"age\": 18,\n"
            "    \"happy\": false,\n"
            "    \"scores\": [65.5]\n"
            "  },\n"
            "  {\n"
            "    \"type\": \"exchange\",\n"
            "    \"name\": \"David\",\n"
            "    \"age\": 21,\n"
            "    \"happy\": true,\n"
            "    \"scores\": [99, 97.5]\n"
            "  }\n"
            "]";
        auto result_json = parseJson(str);
        spargel_check(result_json.isLeft());

        auto result = studentCodec.arrayOf().decode(decodeBackend, base::move(result_json.left()));
        spargel_check(result.isLeft());

        auto& students = result.left();

        spargel_check(students[0].type == base::string("normal"));
        spargel_check(students[0].name == base::string("Alice"));
        spargel_check(!students[0].nickname.hasValue());
        spargel_check(students[0].age == 20);
        spargel_check(students[0].happy == true);
        spargel_check(students[0].scores.count() == 3);
        spargel_check(fabs(students[0].scores[0] - 98) < 1e-6f);
        spargel_check(fabs(students[0].scores[1] - 87.5f) < 1e-6f);
        spargel_check(fabs(students[0].scores[2] - 92) < 1e-6f);

        spargel_check(students[1].type == base::string("normal"));
        spargel_check(students[1].name == base::string("Bob"));
        spargel_check(students[1].nickname.hasValue() &&
                      students[1].nickname.value() == base::string("Bomb"));
        spargel_check(students[1].age == 18);
        spargel_check(students[1].happy == false);
        spargel_check(students[1].scores.count() == 1);
        spargel_check(fabs(students[1].scores[0] - 65.5) < 1e-6f);

        spargel_check(students[2].type == base::string("exchange"));
        spargel_check(students[2].name == base::string("David"));
        spargel_check(!students[2].nickname.hasValue());
        spargel_check(students[2].age == 21);
        spargel_check(students[2].happy == true);
        spargel_check(students[2].scores.count() == 2);
        spargel_check(fabs(students[2].scores[0] - 99) < 1e-6f);
        spargel_check(fabs(students[2].scores[1] - 97.5f) < 1e-6f);
    }
}

TEST(Json_Codec_Decode_Record_FailFast) {
    // TODO
}
