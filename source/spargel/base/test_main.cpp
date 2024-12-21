#include <spargel/base/test.h>

int main() {
    spargel::base::TestManager::instance()->runAll();
    return 0;
}
