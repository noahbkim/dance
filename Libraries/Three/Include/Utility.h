#include <cmath>

namespace Dance::Three::Math
{
    const float Pi = 3.1415926535f;
    const float TwoPi = Pi * 2.0f;
    const float PiOver2 = Pi / 2.0f;

    inline float ToRadians(float degrees)
    {
        return degrees * Pi / 180.0f;
    }

    inline float ToDegrees(float radians)
    {
        return radians * 180.0f / Pi;
    }

    template <typename T>
    inline T Max(const T& a, const T& b)
    {
        return (a < b ? b : a);
    }

    template <typename T>
    inline T Min(const T& a, const T& b)
    {
        return (a < b ? a : b);
    }

    template <typename T>
    inline T Clamp(const T& value, const T& lower, const T& upper)
    {
        return Min(upper, Max(lower, value));
    }

    inline float Lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

    inline bool IsZero(float val, float epsilon = 0.001f)
    {
        return fabs(val) <= epsilon;
    }

    inline bool IsNear(float a, float b, float epsilon = 0.001f)
    {
        return fabsf(a - b) <= epsilon * Max(Max(1.0f, fabsf(a)), fabsf(b));
    }
}
