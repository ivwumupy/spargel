#pragma once

#include <spargel/base/attribute.h>

//
#include <math.h>

namespace spargel::math {

    SPARGEL_ALWAYS_INLINE float abs(float x) { return ::fabsf(x); }
    SPARGEL_ALWAYS_INLINE double abs(double x) { return ::fabs(x); }

    SPARGEL_ALWAYS_INLINE float sqrt(float x) { return ::sqrtf(x); }
    SPARGEL_ALWAYS_INLINE double sqrt(double x) { return ::sqrt(x); }

    SPARGEL_ALWAYS_INLINE float tan(float x) { return ::tanf(x); }

}
