#pragma once

#include <cmath>

namespace Dance::Three::Math
{
    template<typename T>
    class Vector2
    {
    public:
        T x;
        T y;

        Vector2() : x(0), y(0) {}
        explicit Vector2(T x, T y) : x(x), y(y) {}

        friend Vector2 operator+(const Vector2& a, const Vector2& b)
        {
            return Vector2(a.x + b.x, a.y + b.y);
        }

        Vector2& operator+=(const Vector2& v)
        {
            this->x += v.x;
            this->y += v.y;
            return *this;
        }

        friend Vector2 operator-(const Vector2& a, const Vector2& b)
        {
            return Vector2(a.x - b.x, a.y - b.y);
        }

        Vector2& operator-=(const Vector2& v)
        {
            this->x -= v.x;
            this->y -= v.y;
            return *this;
        }

        friend Vector2 operator*(const Vector2& a, const Vector2& b)
        {
            return Vector2(a.x * b.x, a.y * b.y);
        }

        friend Vector2 operator*(const Vector2& v, T s)
        {
            return Vector2(v.x * s, v.y * s);
        }

        friend Vector2 operator*(T s, const Vector2& v)
        {
            return Vector2(v.x * s, v.y * s);
        }

        Vector2& operator*=(T s)
        {
            this->x *= s;
            this->y *= s;
            return *this;
        }

        friend Vector2 operator/(const Vector2& v, T s)
        {
            return Vector2(v.x / s, v.y / s);
        }

        Vector2& operator/=(T s)
        {
            this->x /= s;
            this->y /= s;
            return *this;
        }

        T Magnitude() const
        {
            return this->x * this->x + this->y * this->y;
        }

        T Length() const
        {
            return std::sqrt(this->Magnitude());
        }

        void Normalize()
        {
            *this /= this->Length();
        }

        Vector2 Normalized() const
        {
            Vector2 result(*this);
            result.Normalize();
            return result;
        }

        template<typename T>
        static T Dot(const Vector2<T>& a, const Vector2<T>& b)
        {
            return a.x * b.x + a.y * b.y;
        }

        template<typename T>
        static Vector2<T> Lerp(const Vector2<T>& a, const Vector2<T>& b, T f)
        {
            return a + f * (b - a);
        }

        template<typename T>
        static constexpr Vector2<T> Zero() { return Vector2<T>(0, 0); }

        template<typename T>
        static constexpr Vector2<T> One() { return Vector2<T>(1, 1); }

        template<typename T>
        static constexpr Vector2<T> X() { return Vector2<T>(1, 0); }

        template<typename T>
        static constexpr Vector2<T> Y() { return Vector2<T>(0, 1); }
    };

    using Vector2F = Vector2<float>;
    using Vector2D = Vector2<double>;
}
