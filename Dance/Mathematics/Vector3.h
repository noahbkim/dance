#pragma once

#include "Utility.h"

#include <cstring>
#include <xmmintrin.h>

template<typename T>
class Vector3
{
public:
    T x;
    T y;
    T z;

    Vector3() : x(0), y(0), z(0) {}
    explicit Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

#pragma warning(push)
#pragma warning(disable : 26495)
    explicit Vector3(__m128 value)
    {
        memcpy(this, &value, sizeof(T) * 3);
    }
#pragma warning(pop)

    friend Vector3 operator+(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    Vector3& operator+=(const Vector3& v)
    {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return *this;
    }

    friend Vector3 operator-(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    Vector3& operator-=(const Vector3& v)
    {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        return *this;
    }

    friend Vector3 operator*(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
    }

    friend Vector3 operator*(const Vector3& v, T s)
    {
        return Vector3(v.x * s, v.y * s, v.z * s);
    }

    friend Vector3 operator*(T s, const Vector3& v)
    {
        return Vector3(v.x * s, v.y * s, v.z * s);
    }

    Vector3& operator*=(T s)
    {
        this->x *= s;
        this->y *= s;
        this->z *= s;
        return *this;
    }

    friend Vector3 operator/(const Vector3& v, T s)
    {
        return Vector3(v.x / s, v.y / s, v.z / s);
    }

    Vector3& operator/=(T s)
    {
        this->x /= s;
        this->y /= s;
        this->z /= s;
        return *this;
    }

    T Magnitude() const
    {
        return this->x * this->x + this->y * this->y + this->z * this->z;
    }

    T Length() const
    {
        return std::sqrt(this->Magnitude());
    }

    void Normalize()
    {
        *this /= this->Length();
    }

    Vector3 Normalized() const
    {
        Vector3 result(*this);
        result.Normalize();
        return result;
    }

    friend T Dot(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    friend Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x), a.x * b.y - a.y * b.x);
    }

    friend Vector3 Lerp(const Vector3& a, const Vector3& b, T f)
    {
        return a + f * (b - a);
    }

    template<typename T>
    static constexpr Vector3<T> Zero() { return Vector3<T>(0, 0, 0); }

    template<typename T>
    static constexpr Vector3<T> One() { return Vector3<T>(1, 1, 1); }

    template<typename T>
    static constexpr Vector3<T> X() { return Vector3<T>(1, 0, 0); }

    template<typename T>
    static constexpr Vector3<T> Y() { return Vector3<T>(0, 1, 0); }

    template<typename T>
    static constexpr Vector3<T> Z() { return Vector3<T>(0, 0, 1); }
};

using Vector3F = Vector3<float>;
using Vector3D = Vector3<double>;
