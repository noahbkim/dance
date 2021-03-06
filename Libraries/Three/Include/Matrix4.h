#pragma once

#include "Vector3.h"
#include "Quaternion.h"

namespace Dance::Three::Math
{
    template<typename T>
    class Matrix4
    {
    public:
        T value[4][4];

        Matrix4()
        {
            *this = Matrix4::Identity<T>();
        }

        explicit Matrix4
        (
            T a, T b, T c, T d,
            T e, T f, T g, T h,
            T i, T j, T k, T l,
            T m, T n, T o, T p
        )
            : value{ {a, b, c, d}, {e, f, g, h}, {i, j, k, l}, {m, n, o, p} }
        {}

        explicit Matrix4(const T value[4][4])
        {
            memcpy(this->value, value, 16 * sizeof(T));
        }

        explicit Matrix4(const __m128 rows[4])
        {
            memcpy(this->value, rows, 16 * sizeof(T));
        }

        const T* Flatten() const
        {
            return reinterpret_cast<const T*>(&this->value[0][0]);
        }

        friend Matrix4 operator*(const Matrix4& a, const Matrix4& b)
        {
            Matrix4 c;

            c.value[0][0] =
                a.value[0][0] * b.value[0][0] +
                a.value[0][1] * b.value[1][0] +
                a.value[0][2] * b.value[2][0] +
                a.value[0][3] * b.value[3][0];
            c.value[0][1] =
                a.value[0][0] * b.value[0][1] +
                a.value[0][1] * b.value[1][1] +
                a.value[0][2] * b.value[2][1] +
                a.value[0][3] * b.value[3][1];
            c.value[0][2] =
                a.value[0][0] * b.value[0][2] +
                a.value[0][1] * b.value[1][2] +
                a.value[0][2] * b.value[2][2] +
                a.value[0][3] * b.value[3][2];
            c.value[0][3] =
                a.value[0][0] * b.value[0][3] +
                a.value[0][1] * b.value[1][3] +
                a.value[0][2] * b.value[2][3] +
                a.value[0][3] * b.value[3][3];

            // row 1
            c.value[1][0] =
                a.value[1][0] * b.value[0][0] +
                a.value[1][1] * b.value[1][0] +
                a.value[1][2] * b.value[2][0] +
                a.value[1][3] * b.value[3][0];
            c.value[1][1] =
                a.value[1][0] * b.value[0][1] +
                a.value[1][1] * b.value[1][1] +
                a.value[1][2] * b.value[2][1] +
                a.value[1][3] * b.value[3][1];
            c.value[1][2] =
                a.value[1][0] * b.value[0][2] +
                a.value[1][1] * b.value[1][2] +
                a.value[1][2] * b.value[2][2] +
                a.value[1][3] * b.value[3][2];
            c.value[1][3] =
                a.value[1][0] * b.value[0][3] +
                a.value[1][1] * b.value[1][3] +
                a.value[1][2] * b.value[2][3] +
                a.value[1][3] * b.value[3][3];
            // row 2
            c.value[2][0] =
                a.value[2][0] * b.value[0][0] +
                a.value[2][1] * b.value[1][0] +
                a.value[2][2] * b.value[2][0] +
                a.value[2][3] * b.value[3][0];
            c.value[2][1] =
                a.value[2][0] * b.value[0][1] +
                a.value[2][1] * b.value[1][1] +
                a.value[2][2] * b.value[2][1] +
                a.value[2][3] * b.value[3][1];
            c.value[2][2] =
                a.value[2][0] * b.value[0][2] +
                a.value[2][1] * b.value[1][2] +
                a.value[2][2] * b.value[2][2] +
                a.value[2][3] * b.value[3][2];
            c.value[2][3] =
                a.value[2][0] * b.value[0][3] +
                a.value[2][1] * b.value[1][3] +
                a.value[2][2] * b.value[2][3] +
                a.value[2][3] * b.value[3][3];

            // row 3
            c.value[3][0] =
                a.value[3][0] * b.value[0][0] +
                a.value[3][1] * b.value[1][0] +
                a.value[3][2] * b.value[2][0] +
                a.value[3][3] * b.value[3][0];
            c.value[3][1] =
                a.value[3][0] * b.value[0][1] +
                a.value[3][1] * b.value[1][1] +
                a.value[3][2] * b.value[2][1] +
                a.value[3][3] * b.value[3][1];
            c.value[3][2] =
                a.value[3][0] * b.value[0][2] +
                a.value[3][1] * b.value[1][2] +
                a.value[3][2] * b.value[2][2] +
                a.value[3][3] * b.value[3][2];
            c.value[3][3] =
                a.value[3][0] * b.value[0][3] +
                a.value[3][1] * b.value[1][3] +
                a.value[3][2] * b.value[2][3] +
                a.value[3][3] * b.value[3][3];

            return c;
        }

        Matrix4& operator*=(const Matrix4& right)
        {
            *this = *this * right;
            return *this;
        }

        void Invert()
        {
            T tmp[12];  // temp array for pairs
            T src[16];  // array of transpose source matrix
            T dst[16];  // storage
            T det;      // determinant

            // row 1 to col 1
            src[0] = this->value[0][0];
            src[4] = this->value[0][1];
            src[8] = this->value[0][2];
            src[12] = this->value[0][3];

            // row 2 to col 2
            src[1] = this->value[1][0];
            src[5] = this->value[1][1];
            src[9] = this->value[1][2];
            src[13] = this->value[1][3];

            // row 3 to col 3
            src[2] = this->value[2][0];
            src[6] = this->value[2][1];
            src[10] = this->value[2][2];
            src[14] = this->value[2][3];

            // row 4 to col 4
            src[3] = this->value[3][0];
            src[7] = this->value[3][1];
            src[11] = this->value[3][2];
            src[15] = this->value[3][3];

            // calculate pairs for first 8 elements (cofactors)
            tmp[0] = src[10] * src[15];
            tmp[1] = src[11] * src[14];
            tmp[2] = src[9] * src[15];
            tmp[3] = src[11] * src[13];
            tmp[4] = src[9] * src[14];
            tmp[5] = src[10] * src[13];
            tmp[6] = src[8] * src[15];
            tmp[7] = src[11] * src[12];
            tmp[8] = src[8] * src[14];
            tmp[9] = src[10] * src[12];
            tmp[10] = src[8] * src[13];
            tmp[11] = src[9] * src[12];

            // calculate first 8 elements (cofactors)
            dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
            dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
            dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
            dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
            dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
            dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
            dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
            dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
            dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
            dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
            dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
            dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
            dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
            dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
            dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
            dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

            // calculate pairs for second 8 elements (cofactors)
            tmp[0] = src[2] * src[7];
            tmp[1] = src[3] * src[6];
            tmp[2] = src[1] * src[7];
            tmp[3] = src[3] * src[5];
            tmp[4] = src[1] * src[6];
            tmp[5] = src[2] * src[5];
            tmp[6] = src[0] * src[7];
            tmp[7] = src[3] * src[4];
            tmp[8] = src[0] * src[6];
            tmp[9] = src[2] * src[4];
            tmp[10] = src[0] * src[5];
            tmp[11] = src[1] * src[4];

            // calculate second 8 elements (cofactors)
            dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
            dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
            dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
            dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
            dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
            dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
            dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
            dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
            dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
            dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
            dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
            dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
            dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
            dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
            dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
            dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

            // calculate determinant
            det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

            // calculate matrix inverse
            det = 1 / det;
            for (int j = 0; j < 16; j++)
            {
                dst[j] *= det;
            }

            // set it back
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    this->value[i][j] = dst[i * 4 + j];
                }
            }
        }

        Matrix4 Inverted() const
        {
            Matrix4 result(*this);
            result.Invert();
            return result;
        }

        Vector3<T> Translation() const
        {
            return Vector3<T>(this->value[3][0], this->value[3][1], this->value[3][2]);
        }

        Vector3<T> X() const
        {
            return Vector3<T>(this->value[0][0], this->value[0][1], this->value[0][2]);
        }

        Vector3<T> Y() const
        {
            return Vector3<T>(this->value[1][0], this->value[1][1], this->value[1][2]);
        }

        Vector3<T> Z() const
        {
            return Vector3<T>(this->value[2][0], this->value[2][1], this->value[2][2]);
        }

        Vector3<T> W() const
        {
            return Vector3<T>(this->value[3][0], this->value[3][1], this->value[3][2]);
        }

        Vector3<T> Scale() const
        {
            return Vector3<T>
                (
                    Vector3(this->value[0][0], this->value[0][1], this->value[0][2]).Length(),
                    Vector3(this->value[1][0], this->value[1][1], this->value[1][2]).Length(),
                    Vector3(this->value[2][0], this->value[2][1], this->value[2][2]).Length()
                    );
        }

        void Transpose()
        {
            std::swap(this->value[0][1], this->value[1][0]);
            std::swap(this->value[0][2], this->value[2][0]);
            std::swap(this->value[0][3], this->value[3][0]);
            std::swap(this->value[1][2], this->value[2][1]);
            std::swap(this->value[1][3], this->value[3][1]);
            std::swap(this->value[2][3], this->value[3][2]);
        }

        Matrix4 Transposed() const
        {
            Matrix4 result(*this);
            result.Transpose();
            return result;
        }

        template<typename T>
        static constexpr Matrix4<T> Scale(T x, T y, T z)
        {
            return Matrix4<T>
                (
                    x, 0, 0, 0,
                    0, y, 0, 0,
                    0, 0, z, 0,
                    0, 0, 0, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Scale(const Vector3<T>& v)
        {
            return Matrix4<T>::Scale(v.x, v.y, v.z);
        }

        template<typename T>
        static constexpr Matrix4<T> Scale(T s)
        {
            return Matrix4<T>::Scale(s, s, s);
        }

        template<typename T>
        static constexpr Matrix4<T> XRotation(T theta)
        {
            return Matrix4<T>
                (
                    1, 0, 0, 0,
                    0, std::cos(theta), std::sin(theta), 0,
                    0, -std::sin(theta), std::cos(theta), 0,
                    0, 0, 0, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> YRotation(T theta)
        {
            return Matrix4<T>
                (
                    std::cos(theta), 0, -std::sin(theta), 0,
                    0, 1, 0, 0,
                    std::sin(theta), 0, std::cos(theta), 0,
                    0, 0, 0, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> ZRotation(T theta)
        {
            return Matrix4<T>
                (
                    std::cos(theta), std::sin(theta), 0, 0,
                    -std::sin(theta), std::cos(theta), 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> From(const Quaternion<T>& q)
        {
            Matrix4<T> m;

            m.value[0][0] = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
            m.value[0][1] = 2 * q.x * q.y + 2 * q.w * q.z;
            m.value[0][2] = 2 * q.x * q.z - 2 * q.w * q.y;
            m.value[0][3] = 0;

            m.value[1][0] = 2 * q.x * q.y - 2 * q.w * q.z;
            m.value[1][1] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
            m.value[1][2] = 2 * q.y * q.z + 2 * q.w * q.x;
            m.value[1][3] = 0;

            m.value[2][0] = 2 * q.x * q.z + 2 * q.w * q.y;
            m.value[2][1] = 2 * q.y * q.z - 2 * q.w * q.x;
            m.value[2][2] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
            m.value[2][3] = 0;

            m.value[3][0] = 0;
            m.value[3][1] = 0;
            m.value[3][2] = 0;
            m.value[3][3] = 1;

            return m;
        }

        template<typename T>
        static constexpr Matrix4<T> Translation(T x, T y, T z)
        {
            return Matrix4<T>
                (
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    x, y, z, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Translation(const Vector3<T>& v)
        {
            return Matrix4<T>
                (
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    v.x, v.y, v.z, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Look(const Vector3<T>& eye, const Vector3<T>& at, const Vector3<T>& up)
        {
            Vector3<T> forward = (at - eye).Normalized();
            Vector3<T> left = Vector3::Cross<T>(up, forward).Normalized();
            Vector3<T> newUp = Vector3::Cross<T>(forward, left).Normalized();

            return Matrix4<T>
                (
                    left.x, left.y, left.z, 0,
                    newUp.x, newUp.y, newUp.z, 0,
                    forward.x, forward.y, forward.z, 0,
                    eye.x, eye.y, eye.z, 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Orthographic(T width, T height, T nearZ, T farZ)
        {
            return Matrix4<T>
                (
                    2 / width, 0, 0, 0,
                    0, 2 / height, 0, 0,
                    0, 0, 1 / (farZ - nearZ), 0,
                    0, 0, nearZ / (nearZ - farZ), 1
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Perspective(T fovY, T width, T height, T nearZ, T farZ)
        {
            T yScale = 1 / std::tan(fovY / 2);
            T xScale = yScale * height / width;
            return Matrix4<T>
                (
                    xScale, 0, 0, 0,
                    0, yScale, 0, 0,
                    0, 0, farZ / (farZ - nearZ), 1,
                    0, 0, -nearZ * farZ / (farZ - nearZ), 0
                    );
        }

        template<typename T>
        static constexpr Matrix4<T> Identity()
        {
            return Matrix4<T>
                (
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                    );
        }
    };

    using Matrix4F = Matrix4<float>;
    using Matrix4D = Matrix4<double>;
}
