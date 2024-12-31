#include <spargel/base/tagged_union.h>

enum class FooTag {
    Bar,
    Baz
};

struct Bar {};

struct Baz;

using Foo = spargel::base::TaggedUnion<FooTag>;

int main() { return 0; }
