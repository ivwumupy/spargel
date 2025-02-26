#pragma once

#include <spargel/base/types.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/resource/resource.h>

#if SPARGEL_FILE_MMAP

#if SPARGEL_IS_WINDOWS
typedef void* HANDLE;
#endif

#else

// libc
#include <stdio.h>

#endif  // !SPARGEL_FILE_MMAP

namespace spargel::resource {

    class ResourceManagerDirectory : public ResourceManager {
    public:
        ResourceManagerDirectory(base::string_view root_path) : _root_path(root_path) {}

        bool has(const ResourceId& id) override;

        base::Optional<base::unique_ptr<Resource>> open(const ResourceId& id) override;

    private:
        base::string _root_path;

        base::string _real_path(const ResourceId& id);
    };

    base::unique_ptr<ResourceManagerDirectory> makeRelativeManager(
        const base::string& resources_dir = base::string());

}  // namespace spargel::resource
