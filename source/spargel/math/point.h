#pragma once

namespace spargel::math {

    template <typename T>
    struct Vector3;

    template <typename T>
    struct Point3 {
        T x;
        T y;
        T z;

        Vector3<T> asVector() const { return Vector3<T>(x, y, z); }
    };

    template <typename T>
    Vector3<T> operator-(Point3<T> const& lhs, Point3<T> const& rhs) {
        return Vector3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }

    template <typename T>
    Point3<T> operator+(Point3<T> const& lhs, Vector3<T> const& rhs) {
        return Point3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }

    using Point3f = Point3<float>;

}  // namespace spargel::math
