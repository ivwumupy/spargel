#include <spargel/codec/model/gltf.h>

/* libc */
#include <stdio.h>

using namespace spargel;
using namespace spargel::codec;
using namespace spargel::codec::model;

namespace {

    void dumpGlTF(const GlTF& gltf) {
        // asset
        auto& asset = gltf.asset;
        puts("asset:");
        printf("  version: \"%s\"\n", asset.version.data());
        if (asset.copyright.hasValue())
            printf("  copyright: \"%s\"\n", asset.copyright.value().data());
        if (asset.generator.hasValue())
            printf("  generator: \"%s\"\n", asset.generator.value().data());
        if (asset.minVersion.hasValue())
            printf("  minVersion: \"%s\"\n", asset.minVersion.value().data());

        if (gltf.buffers.hasValue()) printf("buffers: %zu\n", gltf.buffers.value().count());

        if (gltf.bufferViews.hasValue())
            printf("bufferViews: %zu\n", gltf.bufferViews.value().count());

        if (gltf.nodes.hasValue()) printf("nodes: %zu\n", gltf.nodes.value().count());

        if (gltf.scenes.hasValue()) printf("scenes: %zu\n", gltf.scenes.value().count());

        if (gltf.scene.hasValue()) printf("scene: %d\n", gltf.scene.value());
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

    GlTF gltf;
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
