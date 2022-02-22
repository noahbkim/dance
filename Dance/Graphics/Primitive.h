#pragma once

#include "Mathematics/Mathematics.h"

/// Convenience struct for dealing with RGBA colors.
/// 
/// @typeparam T underlying value type for R, G, B, and A.
template<typename T = float>
struct Color4
{
    T r;
    T g;
    T b;
    T a;

    /// Default construct as black with opacity 1.
    Color4() : r(0), g(0), b(0), a(1) {}

    /// Construct exact color, optional opacity defaults to 1.
    /// 
    /// @param r is the red value.
    /// @param g is the green value.
    /// @param b is the blue value.
    /// @param a is the transparency value.
    Color4(T r, T g, T b, T a = static_cast<T>(1)) : r(r), g(g), b(b), a(a) {}
};

/// Float color is what we'll use by default.
using Color4F = Color4<float>;

namespace SimpleVertex
{
    /// Basic vertex structure with position, normal, color, and texture mapping.
    struct Structure
    {
        Vector3F Position;
        Vector3F Normal;
        Color4F Color;
        float UV[2];
    };

    /// Corresponding DirectX mapping for translation to shader data.
    const static D3D11_INPUT_ELEMENT_DESC LAYOUT[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Structure, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Structure, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Structure, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, offsetof(Structure, UV),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
}
