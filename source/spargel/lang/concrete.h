#pragma once

#include <spargel/base/string.h>

namespace spargel::lang::concrete {

    struct Node {
        virtual base::String shortDesc() = 0;
    };

    struct SourceFile : Node {
        base::String shortDesc() override {
            return "SourceFile";
        }
    };

}
