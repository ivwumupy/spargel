#include <spargel/base/assert.h>
#include <spargel/base/inline_array.h>

using namespace spargel;

int main() {
    base::InlineArray<int, 3> x = {1, 2, 3};
    spargel_assert(x.count() == 3);
    spargel_assert(x[0] == 1);
    spargel_assert(x[1] == 2);
    spargel_assert(x[2] == 3);
    return 0;
}
