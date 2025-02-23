#pragma once

#include <spargel/base/types.h>
#include <spargel/resource/resource.h>

// Android
#include <android/asset_manager.h>

namespace spargel::resource {

    class ResourceAndroidAsset : public Resource {
        friend class ResourceManagerAndroidAsset;

    public:
        ~ResourceAndroidAsset() override;

        usize size() override;

        void getData(void* buf) override;

    private:
        AAsset* _asset;

        explicit ResourceAndroidAsset(AAsset* asset) : _asset(asset) {}
    };

    class ResourceManagerAndroidAsset : public ResourceManager {
    public:
        explicit ResourceManagerAndroidAsset(AAssetManager* asset_manager)
            : _asset_manager(asset_manager) {}

        ResourceAndroidAsset* open(const ResourceId& id) override;

    private:
        AAssetManager* _asset_manager;
    };

}  // namespace spargel::resource
