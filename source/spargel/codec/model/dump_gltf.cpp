#include <spargel/codec/model/gltf.h>

/* libc */
#include <stdio.h>

using namespace spargel;
using namespace spargel::codec;
using namespace spargel::codec::model;

namespace {

    void indentWithSpaces(usize indent) {
        for (usize i = 0; i < indent; i++) putchar(' ');
    }

    void dumpIndices(const base::vector<u32>& indices) {
        putchar('[');
        putchar(' ');
        usize count = indices.count();
        for (usize i = 0; i < count; i++) {
            printf("%u", indices[i]);
            if (i < count - 1) {
                putchar(',');
                putchar(' ');
            }
        }
        putchar(' ');
        putchar(']');
    }

    void dumpMatrix(const math::Matrix4x4f& matrix) {
        putchar('[');
        putchar(' ');
        for (usize i = 0; i < 16; i++) {
            printf("%lf", matrix.entries[i]);
            if (i < 16 - 1) {
                putchar(',');
                putchar(' ');
            }
        }
        putchar(' ');
        putchar(']');
    }

    void dumpGlTFAsset(const GlTFAsset& asset, usize indent = 0) {
        // version
        indentWithSpaces(indent);
        printf("version: \"%s\"\n", asset.version.data());

        // [minVersion]
        if (asset.minVersion.hasValue()) {
            indentWithSpaces(indent);
            printf("minVersion: \"%s\"\n", asset.minVersion.value().data());
        }
    }

    void dumpScene(const GlTFScene& scene, usize indent = 0) {
        // [name]
        if (scene.name.hasValue()) {
            indentWithSpaces(indent);
            printf("name: \"%s\"\n", scene.name.value().data());
        }

        // nodes
        indentWithSpaces(indent);
        printf("nodes: ");

        dumpIndices(scene.nodes);

        puts("");
    }

    void dumpNode(const GlTFNode& node, usize indent = 0) {
        // [name]
        if (node.name.hasValue()) {
            indentWithSpaces(indent);
            printf("name: \"%s\"\n", node.name.value().data());
        }

        // [children]
        if (node.children.hasValue()) {
            indentWithSpaces(indent);
            printf("children: ");

            dumpIndices(node.children.value());

            puts("");
        }

        // [matrix]
        if (node.matrix.hasValue()) {
            indentWithSpaces(indent);
            printf("matrix: ");

            dumpMatrix(node.matrix.value());

            puts("");
        }
    }

    void dumpGlTF(const GlTFObject& gltf, usize indent = 0) {
        // asset
        indentWithSpaces(indent);
        puts("asset:");

        dumpGlTFAsset(gltf.asset, indent + 2);

        // [scenes]
        if (gltf.scenes.hasValue()) {
            indentWithSpaces(indent);
            puts("scenes: [");

            usize count = gltf.scenes.value().count();
            for (usize i = 0; i < count; i++) {
                indentWithSpaces(indent + 2);
                printf("%zu: {\n", i);

                dumpScene(gltf.scenes.value()[i], indent + 4);

                indentWithSpaces(indent + 2);
                putchar('}');

                if (i < count - 1) putchar(',');

                puts("");
            }

            indentWithSpaces(indent);
            puts("]");
        }

        // [scene]
        if (gltf.scene.hasValue()) {
            indentWithSpaces(indent);
            printf("scene: %u\n", gltf.scene.value());
        }

        // [nodes]
        if (gltf.scenes.hasValue()) {
            indentWithSpaces(indent);
            puts("nodes: [");

            usize count = gltf.nodes.value().count();
            for (usize i = 0; i < count; i++) {
                indentWithSpaces(indent + 2);
                printf("%zu: {\n", i);

                dumpNode(gltf.nodes.value()[i], indent + 4);

                indentWithSpaces(indent + 2);
                putchar('}');

                if (i < count - 1) putchar(',');

                puts("");
            }

            indentWithSpaces(indent);
            puts("]");
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

    JsonValue json;
    auto json_result = parseJson(data, len, json);
    if (json_result.failed()) {
        fprintf(stderr, "Failed to parse JSON: %s\n", json_result.message().data());
        base::default_allocator()->free(data, len);
        return 1;
    }

    GlTFObject gltf;
    auto result = parseGlTF(json, gltf);
    if (result.failed()) {
        fprintf(stderr, "Failed to parse glTF: %s\n", result.message().data());
        base::default_allocator()->free(data, len);
        return 1;
    }

    dumpGlTF(gltf);

    base::default_allocator()->free(data, len);

    return 0;
}
