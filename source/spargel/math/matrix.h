#pragma once

#include <spargel/base/attribute.h>

namespace spargel::math {

    /// 4x4 matrix.
    ///
    /// The storage is column-major.
    ///
    template <typename T>
    struct Matrix4x4 {
        // column-major
        // T entries[4][4];
        T entries[16];

        Matrix4x4() = default;

        // clang-format off
        Matrix4x4(T a11, T a21, T a31, T a41,
                  T a12, T a22, T a32, T a42,
                  T a13, T a23, T a33, T a43,
                  T a14, T a24, T a34, T a44)
            // : entries{{a11, a21, a31, a41},
            //           {a12, a22, a32, a42},
            //           {a13, a23, a33, a43},
            //           {a14, a24, a34, a44}} {}
            : entries{a11, a21, a31, a41,
                      a12, a22, a32, a42,
                      a13, a23, a33, a43,
                      a14, a24, a34, a44} {}
        // clang-format on

        T& operator()(int row, int col) { return entries[4 * col + row]; }
        const T& operator()(int row, int col) const { return entries[4 * col + row]; }
    };

    template <typename T>
    Matrix4x4<T> operator*(const Matrix4x4<T>& m1, const Matrix4x4<T>& m2) {
        Matrix4x4<T> m;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m(i, j) = m1(i, 0) * m2(0, j) + m1(i, 1) * m2(1, j) + m1(i, 2) * m2(2, j) + m1(i, 3) * m2(3, j);
            }
        }
        return m;
    }

    using Matrix4x4f = Matrix4x4<float>;

    template <typename T>
    struct Matrix3x3 {
        T entries[9];

        Matrix3x3() = default;

        Matrix3x3(T a11, T a21, T a31,
                  T a12, T a22, T a32,
                  T a13, T a23, T a33)
            : entries{a11, a21, a31,
                      a12, a22, a32,
                      a13, a23, a33} {}

        T& operator()(int row, int col) { return entries[3 * col + row]; }
        const T& operator()(int row, int col) const { return entries[3 * col + row]; }
    };

    template <typename T>
    Matrix3x3<T> operator*(const Matrix3x3<T>& m1, const Matrix3x3<T>& m2) {
        Matrix3x3<T> m;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                m(i, j) = m1(i, 0) * m2(0, j) + m1(i, 1) * m2(1, j) + m1(i, 2) * m2(2, j);
            }
        }
        return m;
    }

    using Matrix3x3f = Matrix3x3<float>;

}  // namespace spargel::math
