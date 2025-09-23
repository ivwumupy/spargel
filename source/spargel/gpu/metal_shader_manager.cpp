#include "spargel/gpu/metal_shader_manager.h"

namespace spargel::gpu {

    MetalShaderManager& MetalShaderManager::instance() {
        static MetalShaderManager manager;
        return manager;
    }

    void MetalShaderManager::registerLibrary(MetalLibraryMeta const& meta) {
        libraries_.push(meta);
    }

    base::Result<MetalShaderManager::QueryResult,
                 MetalShaderManager::QueryError>
    MetalShaderManager::queryShader(base::StringView shader_id) {
        for (auto const& lib : libraries_) {
            for (auto const& func : lib.functions) {
                if (func.shader_id == shader_id) {
                    return QueryResult{lib.path, func};
                }
            }
        }
        return base::Error{QueryError::not_found};
    }

}  // namespace spargel::gpu
