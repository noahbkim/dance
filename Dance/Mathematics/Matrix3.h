#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include <cstring>
#include <cmath>
#include <algorithm>

template<typename T>
class Matrix3
{
public:
    T value[3][3];

    Matrix3()
    {
        *this = Matrix3::Identity();
    }

    explicit Matrix3(const T value[3][3])
    {
        std::memcpy(this->value, value, 9 * sizeof(T));
    }

    const T* Flatten() const
    {
        return reinterpret_cast<const T*>(&this->value[0][0]);
    }

    friend Matrix3 operator*(const Matrix3& a, const Matrix3& b)
    {
        Matrix3<T> c;
        c.value[0][0] =
            a.value[0][0] * b.value[0][0] +
            a.value[0][1] * b.value[1][0] +
            a.value[0][2] * b.value[2][0];

        c.value[0][1] =
            a.value[0][0] * b.value[0][1] +
            a.value[0][1] * b.value[1][1] +
            a.value[0][2] * b.value[2][1];

        c.value[0][2] =
            a.value[0][0] * b.value[0][2] +
            a.value[0][1] * b.value[1][2] +
            a.value[0][2] * b.value[2][2];

        // row 1
        c.value[1][0] =
            a.value[1][0] * b.value[0][0] +
            a.value[1][1] * b.value[1][0] +
            a.value[1][2] * b.value[2][0];

        c.value[1][1] =
            a.value[1][0] * b.value[0][1] +
            a.value[1][1] * b.value[1][1] +
            a.value[1][2] * b.value[2][1];

        c.value[1][2] =
            a.value[1][0] * b.value[0][2] +
            a.value[1][1] * b.value[1][2] +
            a.value[1][2] * b.value[2][2];

        // row 2
        c.value[2][0] =
            a.value[2][0] * b.value[0][0] +
            a.value[2][1] * b.value[1][0] +
            a.value[2][2] * b.value[2][0];

        c.value[2][1] =
            a.value[2][0] * b.value[0][1] +
            a.value[2][1] * b.value[1][1] +
            a.value[2][2] * b.value[2][1];

        c.value[2][2] =
            a.value[2][0] * b.value[0][2] +
            a.value[2][1] * b.value[1][2] +
            a.value[2][2] * b.value[2][2];

        return c;
    }

    Matrix3& operator*=(const Matrix3& m)
    {
        *this = *this * m;
        return *this;
    }

    void Transpose()
    {
        std::swap(this->value[0][1], value[1][0]);
        std::swap(this->value[0][2], value[2][0]);
        std::swap(this->value[1][2], value[2][1]);
    }

    Matrix3 Transposed() const
    {
        Matrix3 result(*this);
        result.Transpose();
        return result;
    }

    template<typename T>
    static constexpr Matrix3<T> Scale(T x, T y)
    {
        return Matrix3<T>
        ({
            { x, 0, 0 },
            { 0, y, 0 },
            { 0, 0, 1 },
        });
    }

    template<typename T>
    static constexpr Matrix3<T> Scale(const Vector2<T>& v)
    {
        return Matrix3::Scale(v.x, v.y);
    }

    template<typename T>
    static constexpr Matrix3<T> Scale(T s)
    {
        return Matrix3::Scale(s, s);
    }

    template<typename T>
    static constexpr Matrix3 Rotation(T theta)
    {
        return Matrix3
        ({
            { std::cos(theta), std::sin(theta), 0 },
            { -std::sin(theta), std::cos(theta), 0 },
            { 0, 0, 1 },
        });
    }

    template<typename T>
    static constexpr Matrix3 Translation(const Vector2<T>& v)
    {
        return Matrix3
        ({
            { 1, 0, 0 },
            { 0, 1, 0 },
            { v.x, v.y, 1 },
        });
    }

    template<typename T>
    static constexpr Matrix3<T> Identity()
    {
        return Matrix3
        ({
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 }
        });
    }
};

using Matrix3F = Matrix3<float>;
using Matrix3D = Matrix3<double>;
