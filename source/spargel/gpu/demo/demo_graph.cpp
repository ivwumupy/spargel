#include <spargel/base/logging.h>
#include <spargel/gpu/gpu.h>

// libc
#include <stdio.h>

using namespace spargel;
using namespace spargel::gpu;
using namespace spargel::base::literals;

struct prepared_graph {
    enum class node_kind {
        render,
        present,
        texture,
        buffer,
    };
    struct node_info {
        node_info(node_kind k, u32 i, base::StringView n) : kind{k}, index{i}, name{n} {}

        node_kind kind;
        u32 index;
        base::StringView name;
        u32 refcount = 0;
        bool culled = false;
        bool target = false;
        base::vector<u32> inputs;
        base::vector<u32> outputs;
    };
    struct render_node {};
    struct present_node {};
    struct texture_node {};
    struct buffer_node {};
    base::vector<node_info> nodes;
    base::vector<render_node> renders;
    base::vector<present_node> presents;
    base::vector<texture_node> textures;
    base::vector<buffer_node> buffers;
};

int main() {
    prepared_graph graph;
    graph.nodes.emplace(prepared_graph::node_kind::texture, 0, "surface.0"_sv);
    graph.nodes.emplace(prepared_graph::node_kind::render, 0, "draw_triangle"_sv);
    graph.nodes.emplace(prepared_graph::node_kind::texture, 1, "surface.1"_sv);
    graph.nodes.emplace(prepared_graph::node_kind::present, 0, "present"_sv);
    graph.nodes.emplace(prepared_graph::node_kind::render, 1, "draw_debug"_sv);
    graph.nodes.emplace(prepared_graph::node_kind::texture, 2, "surface.2"_sv);
    // the <<draw_triangle>> node
    graph.nodes[0].outputs.emplace(1);
    graph.nodes[1].inputs.emplace(0);
    graph.nodes[1].outputs.emplace(2);
    graph.nodes[2].inputs.emplace(1);
    // the <<present>> node
    graph.nodes[2].outputs.emplace(3);
    graph.nodes[3].inputs.emplace(2);
    // the <<debug_draw>> node
    graph.nodes[2].outputs.emplace(4);
    graph.nodes[4].inputs.emplace(2);
    graph.nodes[4].outputs.emplace(5);
    graph.nodes[5].inputs.emplace(4);

    // <<present>> is what we need!
    graph.nodes[3].target = true;

    // print original graph
    printf("original graph:\n");
    for (usize i = 0; i < graph.nodes.count(); i++) {
        auto& node = graph.nodes[i];
        for (usize j = 0; j < node.outputs.count(); j++) {
            auto& other = graph.nodes[node.outputs[j]];
            printf("    <<%s>> ---> <<%s>>\n", node.name.data(), other.name.data());
        }
    }

    // fill initial ref count
    for (usize i = 0; i < graph.nodes.count(); i++) {
        auto& node = graph.nodes[i];
        node.refcount = node.outputs.count();
    }

    printf("begin culling:\n");

    u32 cull_count = 0;

    base::vector<u32> tmp;
    for (usize i = 0; i < graph.nodes.count(); i++) {
        auto& node = graph.nodes[i];
        if (node.refcount == 0 && !node.target) {
            node.culled = true;
            tmp.emplace(i);
            cull_count++;
            printf("    culled node <<%s>>\n", node.name.data());
        }
    }
    while (!tmp.empty()) {
        auto i = tmp[tmp.count() - 1];
        tmp.pop();
        auto& node = graph.nodes[i];
        for (usize j = 0; j < node.inputs.count(); j++) {
            auto& source = graph.nodes[node.inputs[j]];
            source.refcount--;
            if (source.refcount == 0) {
                source.culled = true;
                tmp.emplace(j);
                cull_count++;
                printf("    culled node <<%s>>\n", source.name.data());
            }
        }
    }

    printf("culled %d nodes\n", cull_count);

    printf("culled graph:\n");
    for (usize i = 0; i < graph.nodes.count(); i++) {
        auto& node = graph.nodes[i];
        if (node.culled) continue;
        for (usize j = 0; j < node.outputs.count(); j++) {
            auto& other = graph.nodes[node.outputs[j]];
            if (other.culled) continue;
            printf("    <<%s>> ---> <<%s>>\n", node.name.data(), other.name.data());
        }
    }

    // another approach

    printf("direct approach:\n    ");

    for (usize i = 0; i < graph.nodes.count(); i++) {
        if (graph.nodes[i].target) {
            tmp.emplace(i);
        }
    }
    base::vector<u32> result;
    while (!tmp.empty()) {
        auto i = tmp[tmp.count() - 1];
        tmp.pop();
        result.emplace(i);
        auto& node = graph.nodes[i];
        for (usize j = 0; j < node.inputs.count(); j++) {
            tmp.emplace(node.inputs[j]);
        }
    }

    for (auto iter = result.end() - 1; iter >= result.begin(); iter--) {
        printf("<<%s>>", graph.nodes[*iter].name.data());
        if (iter > result.begin()) {
            printf(" ---> ");
        }
    }
    printf("\n");

    return 0;
}
