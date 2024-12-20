#include <spargel/config.h>
#include <spargel/ui/ui.h>

namespace spargel::ui {

#if SPARGEL_IS_LINUX
    base::unique_ptr<platform> make_platform_xcb();
#elif SPARGEL_IS_MACOS
    base::unique_ptr<platform> make_platform_appkit();
#elif SPARGEL_IS_WINDOWS
    base::unique_ptr<platform> make_platform_win32();
#elif SPARGEL_UI_DUMMY
    base::unique_ptr<platform> make_platform_dummy();
#endif

    base::unique_ptr<platform> make_platform() {
#if SPARGEL_IS_LINUX
        return make_platform_xcb();
#elif SPARGEL_IS_MACOS
        return make_platform_appkit();
#elif SPARGEL_IS_WINDOWS
        return make_platform_win32();
#elif SPARGEL_UI_DUMMY
        return make_platform_dummy();
#else
        return nullptr;
#endif
    }

    char keyboard_event::toChar() {
        switch (key) {
        case physical_key::key_a:
            return 'a';
        case physical_key::key_b:
            return 'b';
        case physical_key::key_c:
            return 'c';
        case physical_key::key_d:
            return 'd';
        case physical_key::key_e:
            return 'e';
        case physical_key::key_f:
            return 'f';
        case physical_key::key_g:
            return 'g';
        case physical_key::key_h:
            return 'h';
        case physical_key::key_i:
            return 'i';
        case physical_key::key_j:
            return 'j';
        case physical_key::key_k:
            return 'k';
        case physical_key::key_l:
            return 'l';
        case physical_key::key_m:
            return 'm';
        case physical_key::key_n:
            return 'n';
        case physical_key::key_o:
            return 'o';
        case physical_key::key_p:
            return 'p';
        case physical_key::key_q:
            return 'q';
        case physical_key::key_r:
            return 'r';
        case physical_key::key_s:
            return 's';
        case physical_key::key_t:
            return 't';
        case physical_key::key_u:
            return 'u';
        case physical_key::key_v:
            return 'v';
        case physical_key::key_w:
            return 'w';
        case physical_key::key_x:
            return 'x';
        case physical_key::key_y:
            return 'y';
        case physical_key::key_z:
            return 'z';
        default:
            return '.';
        }
    }

}  // namespace spargel::ui
