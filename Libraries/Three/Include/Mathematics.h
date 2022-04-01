#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include "Quaternion.h"

namespace Dance::Three::Math
{
    template<typename T>
    inline constexpr Vector2<T> Transform(const Matrix3<T>& m, const Vector2<T>& v, float z = 0.0f)
    {
        return Vector2
        (
            v.x * m.value[0][0] + v.y * m.value[1][0] + z * m.value[2][0],
            v.x * m.value[0][1] + v.y * m.value[1][1] + z * m.value[2][1]
        );
    }

    template<typename T>
    inline constexpr Vector3<T> Transform(const Matrix3<T>& m, const Vector3<T>& v)
    {
        return Vector3
        (
            v.x * m.value[0][0] + v.y * m.value[1][0] + v.z * m.value[2][0],
            v.x * m.value[0][1] + v.y * m.value[1][1] + v.z * m.value[2][1],
            v.x * m.value[0][2] + v.y * m.value[1][2] + v.z * m.value[2][2]
        );
    }

    template<typename T>
    inline constexpr Vector3<T> Transform(const Matrix4<T>& m, const Vector3<T>& v, float w = 1.0f)
    {
        return Vector3<T>
            (
                v.x * m.value[0][0] + v.y * m.value[1][0] + v.z * m.value[2][0] + w * m.value[3][0],
                v.x * m.value[0][1] + v.y * m.value[1][1] + v.z * m.value[2][1] + w * m.value[3][1],
                v.x * m.value[0][2] + v.y * m.value[1][2] + v.z * m.value[2][2] + w * m.value[3][2]
                );
    }

    template<typename T>
    inline constexpr Vector4<T> Transform(const Matrix4<T>& m, const Vector4<T>& v)
    {
        return Vector4
        (
            v.x * m.value[0][0] + v.y * m.value[1][0] + v.z * m.value[2][0] + v.w * m.value[3][0],
            v.x * m.value[0][1] + v.y * m.value[1][1] + v.z * m.value[2][1] + v.w * m.value[3][1],
            v.x * m.value[0][2] + v.y * m.value[1][2] + v.z * m.value[2][2] + v.w * m.value[3][2],
            v.x * m.value[0][3] + v.y * m.value[1][3] + v.z * m.value[2][3] + v.w * m.value[3][3]
        );
    }

    template<typename T>
    inline constexpr Vector3<T> Transform(const Quaternion<T>& q, const Vector3<T>& v)
    {
        Vector3<T> qv = q.Three();
        return v + 2.0f * Vector3::Cross(qv, Cross(qv, v) + q.w * v);
    }
}
