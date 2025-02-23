
#include <spargel/resource/android_asset.h>

namespace spargel::resource {

    ResourceAndroidAsset::~ResourceAndroidAsset() { AAsset_close(_asset); }

    usize ResourceAndroidAsset::size() { return AAsset_getLength(_asset); }

    void ResourceAndroidAsset::getData(void* buf) { AAsset_read(_asset, buf, size()); }

    ResourceAndroidAsset* ResourceManagerAndroidAsset::open(
        const spargel::resource::ResourceId& id) {
        AAsset* asset = AAssetManager_open(_asset_manager, id.path().data(), AASSET_MODE_BUFFER);
        return asset ? new ResourceAndroidAsset(asset) : nullptr;
    }
}  // namespace spargel::resource
