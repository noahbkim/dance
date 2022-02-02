#pragma once

#include <cstring>
#include <cmath>
#include <xmmintrin.h>

template<typename T>
class Vector4
{
public:
    T x;
    T y;
    T z;
    T w;

    Vector4() : x(0), y(0), z(0), w(0) {}
    explicit Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

#pragma warning(push)
#pragma warning(disable : 26495)
    explicit Vector4(__m128 value)
    {
        memcpy(this, &value, sizeof(T) * 4);
    }
#pragma warning(pop)

    friend Vector4 operator+(const Vector4& a, const Vector4& b)
    {
        return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    Vector4& operator+=(const Vector4& v)
    {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        this->w += v.w;
        return *this;
    }

    friend Vector4 operator-(const Vector4& a, const Vector4& b)
    {
        return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }

    Vector4& operator-=(const Vector4& v)
    {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        this->w -= v.w;
        return *this;
    }

    friend Vector4 operator*(const Vector4& a, const Vector4& b)
    {
        return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    }

    friend Vector4 operator*(const Vector4& v, T s)
    {
        return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
    }

    friend Vector4 operator*(T s, const Vector4& v)
    {
        return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
    }

    Vector4& operator*=(T s)
    {
        this->x *= s;
        this->y *= s;
        this->z *= s;
        this->w *= s;
        return *this;
    }

    friend Vector4 operator/(const Vector4& v, T s)
    {
        return Vector4(v.x / s, v.y / s, v.z / s, v.w / s);
    }

    Vector4& operator/=(T s)
    {
        this->x /= s;
        this->y /= s;
        this->z /= s;
        this->w /= s;
        return *this;
    }

    T Magnitude() const
    {
        return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
    }

    T Length() const
    {
        return std::sqrt(this->Magnitude());
    }

    void Normalize()
    {
        *this /= this->Length();
    }

    Vector4 Normalized() const
    {
        Vector4 result(*this);
        result.Normalize();
        return result;
    }

    friend Vector4 Lerp(const Vector4& a, const Vector4& b, T f)
    {
        return a + f * (b - a);
    }

    template<typename T>
    static constexpr Vector4<T> Zero() { return Vector4<T>(0, 0, 0, 0); }

    template<typename T>
    static constexpr Vector4<T> One() { return Vector4<T>(1, 1, 1, 1); }

    template<typename T>
    static constexpr Vector4<T> X() { return Vector4<T>(1, 0, 0, 0); }

    template<typename T>
    static constexpr Vector4<T> Y() { return Vector4<T>(0, 1, 0, 0); }

    template<typename T>
    static constexpr Vector4<T> Z() { return Vector4<T>(0, 0, 1, 0); }

    template<typename T>
    static constexpr Vector4<T> W() { return Vector4<T>(0, 0, 0, 1); }
};

using Vector4F = Vector4<float>;
using Vector4D = Vector4<double>;
