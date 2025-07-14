#pragma once

#include <spargel/base/attribute.h>

//
#include <math.h>

namespace spargel::math {

    inline float abs(float x) { return ::fabsf(x); }
    inline double abs(double x) { return ::fabs(x); }

    inline float sqrt(float x) { return ::sqrtf(x); }
    inline double sqrt(double x) { return ::sqrt(x); }

    inline float tan(float x) { return ::tanf(x); }

}  // namespace spargel::math
