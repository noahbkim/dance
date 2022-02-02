#pragma once

#include "Vector3.h"

template<typename T>
class Quaternion
{
public:
    T x;
    T y;
    T z;
    T w;

    Quaternion() : x(0), y(0), z(0), w(1) {}
    explicit Quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    // Construct the quaternion from an axis and angle
    // It is assumed that axis is already normalized,
    // and the angle is in radians
    explicit Quaternion(const Vector3<T>& axis, T angle)
    {
        T scalar = std::sin(angle / static_cast<T>(2));
        this->x = axis.x * scalar;
        this->y = axis.y * scalar;
        this->z = axis.z * scalar;
        this->w = std::cos(angle / static_cast<T>(2));
    }

    Vector3<T> Three() const
    {
        return Vector3<T>(this->x, this->y, this->z);
    }

    Vector4<T> Four() const
    {
        return Vector4<T>(this->x, this->y, this->z, this->w);
    }

    void Conjugate()
    {
        this->x *= static_cast<T>(-1);
        this->y *= static_cast<T>(-1);
        this->z *= static_cast<T>(-1);
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
        T length = this->Length();
        this->x /= length;
        this->y /= length;
        this->z /= length;
        this->w /= length;
    }

    Quaternion Normalized() const
    {
        Quaternion result(*this);
        result.Normalize();
        return result;
    }

    friend Quaternion Lerp(const Quaternion& a, const Quaternion& b, T f)
    {
        const T dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        const T bias = dot >= static_cast<T>(0) ? static_cast<T>(-1) : static_cast<T>(1);

        Quaternion result
        (
            b.x * f + a.x * bias * (static_cast<T>(1) - f),
            b.y * f + a.y * bias * (static_cast<T>(1) - f),
            b.z * f + a.z * bias * (static_cast<T>(1) - f),
            b.w * f + a.w * bias * (static_cast<T>(1) - f)
        );

        result.Normalize();
        return result;
    }

    // Spherical Linear Interpolation
    friend Quaternion Slerp(const Quaternion& a, const Quaternion& b, T f)
    {
        float rawCosm = Dot(a, b);

        float cosom = -rawCosm;
        if (rawCosm >= 0.0f)
        {
            cosom = rawCosm;
        }

        float scale0, scale1;

        if (cosom < 0.9999f)
        {
            const float omega = acosf(cosom);
            const float invSin = 1.f / sinf(omega);
            scale0 = sinf((1.f - f) * omega) * invSin;
            scale1 = sinf(f * omega) * invSin;
        }
        else
        {
            // Use linear interpolation if the quaternions
            // are collinear
            scale0 = 1.0f - f;
            scale1 = f;
        }

        if (rawCosm < 0.0f)
        {
            scale1 = -scale1;
        }

        Quaternion retVal;
        retVal.x = scale0 * a.x + scale1 * b.x;
        retVal.y = scale0 * a.y + scale1 * b.y;
        retVal.z = scale0 * a.z + scale1 * b.z;
        retVal.w = scale0 * a.w + scale1 * b.w;
        retVal.Normalize();
        return retVal;
    }

    // Concatenate
    // Rotate by q FOLLOWED BY p
    friend Quaternion Concatenate(const Quaternion& q, const Quaternion& p)
    {
        Quaternion retVal;

        // Vector component is:
        // ps * qv + qs * pv + pv x qv
        Vector3 qv(q.x, q.y, q.z);
        Vector3 pv(p.x, p.y, p.z);
        Vector3 newVec = p.w * qv + q.w * pv + Cross(pv, qv);
        retVal.x = newVec.x;
        retVal.y = newVec.y;
        retVal.z = newVec.z;

        // Scalar component is:
        // ps * qs - pv . qv
        retVal.w = p.w * q.w - Dot(pv, qv);

        return retVal;
    }

    // Dot product between two quaternions
    friend T Dot(const Quaternion& a, const Quaternion& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    template<typename T>
    static constexpr Quaternion Identity()
    {
        return Quaternion(0, 0, 0, 1);
    }
};

using QuaternionF = Quaternion<float>;
using QuaternionD = Quaternion<double>;
