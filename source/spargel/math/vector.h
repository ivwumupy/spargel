#pragma once

#include <spargel/math/function.h>

namespace spargel::math {

    template <typename T>
    struct Vector3 {
        T x;
        T y;
        T z;

        SPARGEL_ALWAYS_INLINE Vector3 dot(Vector3 const& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        SPARGEL_ALWAYS_INLINE Vector3 cross(Vector3 const& other) const {
            return Vector3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }


        SPARGEL_ALWAYS_INLINE T length() const {
            return math::sqrt(dot(*this));
        }

        // TODO: What's the behaviour in edge cases?
        SPARGEL_ALWAYS_INLINE void normalize() const {
            T l = length();
            x /= l;
            y /= l;
            z /= l;
        }
    };

    template <typename T>
    SPARGEL_ALWAYS_INLINE Vector3<T> operator+(Vector3<T> const& lhs, Vector3<T> const& rhs) {
        return Vector3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }

    template <typename T>
    SPARGEL_ALWAYS_INLINE Vector3<T> operator-(Vector3<T> const& lhs, Vector3<T> const& rhs) {
        return Vector3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }

    using Vector3f = Vector3<float>;

}
