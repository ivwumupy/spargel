#include <spargel/base/assert.h>
#include <spargel/base/allocator.h>
#include <spargel/codec/codec.h>

namespace spargel::codec {

    void destroy_image(struct image image) {
        if (image.pixels) {
            spargel_assert(image.width > 0 && image.height > 0);
            base::default_allocator()->free(image.pixels,
                                            sizeof(struct color4) * image.width * image.height);
        }
    }

}  // namespace spargel::codec
