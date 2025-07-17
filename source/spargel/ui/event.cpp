#include <spargel/ui/event.h>

namespace spargel::ui {

    char KeyboardEvent::toChar() {
        switch (key) {
        case PhysicalKey::key_0:
            return '0';
        case PhysicalKey::key_1:
            return '1';
        case PhysicalKey::key_2:
            return '2';
        case PhysicalKey::key_3:
            return '3';
        case PhysicalKey::key_4:
            return '4';
        case PhysicalKey::key_5:
            return '5';
        case PhysicalKey::key_6:
            return '6';
        case PhysicalKey::key_7:
            return '7';
        case PhysicalKey::key_8:
            return '8';
        case PhysicalKey::key_9:
            return '9';
        case PhysicalKey::key_a:
            return 'a';
        case PhysicalKey::key_b:
            return 'b';
        case PhysicalKey::key_c:
            return 'c';
        case PhysicalKey::key_d:
            return 'd';
        case PhysicalKey::key_e:
            return 'e';
        case PhysicalKey::key_f:
            return 'f';
        case PhysicalKey::key_g:
            return 'g';
        case PhysicalKey::key_h:
            return 'h';
        case PhysicalKey::key_i:
            return 'i';
        case PhysicalKey::key_j:
            return 'j';
        case PhysicalKey::key_k:
            return 'k';
        case PhysicalKey::key_l:
            return 'l';
        case PhysicalKey::key_m:
            return 'm';
        case PhysicalKey::key_n:
            return 'n';
        case PhysicalKey::key_o:
            return 'o';
        case PhysicalKey::key_p:
            return 'p';
        case PhysicalKey::key_q:
            return 'q';
        case PhysicalKey::key_r:
            return 'r';
        case PhysicalKey::key_s:
            return 's';
        case PhysicalKey::key_t:
            return 't';
        case PhysicalKey::key_u:
            return 'u';
        case PhysicalKey::key_v:
            return 'v';
        case PhysicalKey::key_w:
            return 'w';
        case PhysicalKey::key_x:
            return 'x';
        case PhysicalKey::key_y:
            return 'y';
        case PhysicalKey::key_z:
            return 'z';
        case PhysicalKey::space:
            return ' ';
        default:
            return '.';
        }
    }

}  // namespace spargel::ui
