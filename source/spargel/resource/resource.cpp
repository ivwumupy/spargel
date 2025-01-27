#include <spargel/base/logging.h>
#include <spargel/resource/resource.h>

namespace spargel::resource {

    void resource::close() {
        if (_mapped) {
            base::default_allocator()->free(_mapped, _mapped_size);
        }
    }

    void* resource::map_data() {
        if (!_mapped) {
            _mapped_size = size();
            _mapped = base::default_allocator()->allocate(_mapped_size);
            get_data(_mapped);
        }
        return _mapped;
    }

}  // namespace spargel::resource
