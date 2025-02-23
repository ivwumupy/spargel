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

    class ResourceDirectory : public Resource {
        friend class ResourceManagerDirectory;

    public:
        ~ResourceDirectory() override;

        usize size() override { return _size; }

        void getData(void* buf) override;

        void* mapData() override;

    private:
        usize _size;
#if SPARGEL_FILE_MMAP
        void* _mapped;

#if SPARGEL_IS_POSIX
        int _fd;
        ResourceDirectory(usize size, int fd) : _size(size), _mapped(nullptr), _fd(fd) {}
#elif SPARGEL_IS_WINDOWS
        HANDLE _file_handle;
        HANDLE _mapping_handle;
        ResourceDirectory(usize size, HANDLE file_handle)
            : _size(size), _mapped(nullptr), _file_handle(file_handle), _mapping_handle(nullptr) {}
#else
#error unimplemented
#endif

        void* _map();
        void _unmap(void* ptr, usize size);
#else
        FILE* _fp;
        ResourceDirectory(usize size, FILE* fp) : _size(size), _fp(fp) {}
#endif
    };

    class ResourceManagerDirectory : public ResourceManager {
    public:
        ResourceManagerDirectory(base::string_view root_path) : _root_path(root_path) {}

        ResourceDirectory* open(const ResourceId& id) override;

    private:
        base::string _root_path;

        base::string _real_path(const ResourceId& id);
    };

    base::unique_ptr<ResourceManagerDirectory> makeRelativeManager();

}  // namespace spargel::resource
