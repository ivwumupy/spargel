#pragma once

//
#include <math.h>

namespace spargel::math {

    inline float abs(float x) { return ::fabsf(x); }
    inline double abs(double x) { return ::fabs(x); }

    inline float floor(float x) { return ::floorf(x); }
    inline double floor(double x) { return ::floor(x); }
    inline float ceil(float x) { return ::ceilf(x); }
    inline double ceil(double x) { return ::ceil(x); }
    inline float round(float x) { return ::roundf(x); }
    inline double round(double x) { return ::round(x); }

    inline float sqrt(float x) { return ::sqrtf(x); }
    inline double sqrt(double x) { return ::sqrt(x); }

    inline float cos(float x) { return ::cosf(x); }
    inline float sin(float x) { return ::sinf(x); }
    inline float tan(float x) { return ::tanf(x); }

}  // namespace spargel::math
