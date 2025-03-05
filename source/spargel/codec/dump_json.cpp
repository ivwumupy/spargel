#include <spargel/codec/json.h>

/* libc */
#include <stdio.h>

using namespace spargel;
using namespace spargel::codec;

namespace {

    void dumpValue(const JsonValue& value);

    void dumpObject(const JsonObject& object) { printf("{TODO}"); }

    void dumpArray(const JsonArray& array) {
        putchar('[');
        putchar(' ');
        usize count = array.elements.count();
        for (usize i = 0; i < count; i++) {
            dumpValue(array.elements[i]);
            if (i < count - 1) {
                putchar(',');
                putchar(' ');
            }
        }
        putchar(' ');
        putchar(']');
    }

    void dumpString(const JsonString& string) {
        putchar('"');
        printf("%s", string.data());
        putchar('"');
    }

    void dumpValue(const JsonValue& value) {
        switch (value.type) {
        case JsonValueType::object:
            dumpObject(value.object);
            break;
        case JsonValueType::array:
            dumpArray(value.array);
            break;
        case JsonValueType::string:
            dumpString(value.string);
            break;
        case JsonValueType::number:
            printf("%lf", value.number);
            break;
        case JsonValueType::boolean:
            printf(value.boolean ? "true" : "false");
            break;
        case JsonValueType::null:
            printf("null");
            break;
        }
    }

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // read file
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open file \"%s\"\n", argv[1]);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    ssize len = ftell(fp);
    if (len <= 0) {
        fprintf(stderr, "Bad file size: %zu\n", len);
        fclose(fp);
        return 1;
    }
    char* data = (char*)base::default_allocator()->allocate(len);
    fseek(fp, 0, SEEK_SET);
    fread(data, len, 1, fp);
    fclose(fp);

    auto result = parseJson(data, len);
    if (result.isRight()) {
        fprintf(stderr, "Failed to parse JSON: %s\n", result.right().message().data());
        base::default_allocator()->free(data, len);
        return 1;
    }

    dumpValue(result.left());
    putchar('\n');

    base::default_allocator()->free(data, len);

    return 0;
}
