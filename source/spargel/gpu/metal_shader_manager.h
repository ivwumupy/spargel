#pragma once

#include "spargel/base/functional.h"
#include "spargel/base/result.h"
#include "spargel/base/span.h"
#include "spargel/base/string_view.h"
#include "spargel/base/vector.h"

namespace spargel::gpu {

    struct MetalFunctionMeta {
        base::StringView shader_id;
        base::StringView entry_name;
        // TODO: argument info
    };

    struct MetalLibraryMeta {
        // path to this library
        base::StringView path;
        base::Span<MetalFunctionMeta> functions;
    };

    class MetalShaderManager {
    public:
        struct QueryResult {
            // path to the library
            base::StringView library;
            MetalFunctionMeta meta;
        };
        enum class QueryError {
            not_found,
        };

        static MetalShaderManager& instance();

        void registerLibrary(MetalLibraryMeta const& meta);

        base::Result<QueryResult, QueryError> queryShader(
            base::StringView shader_id);

    private:
        base::Vector<MetalLibraryMeta> libraries_;
    };

}  // namespace spargel::gpu
