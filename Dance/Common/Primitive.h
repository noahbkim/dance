#pragma once

#include "Mathematics.h"

struct Position3
{
	float x;
	float y;
	float z;
};

template<typename T = float>
struct Color4
{
    T r;
    T g;
    T b;
    T a;

    Color4() : r(0), g(0), b(0), a(1) {}
    Color4(T r, T g, T b, T a = static_cast<T>(1)) : r(r), g(g), b(b), a(a) {}
};

using Color4F = Color4<float>;

struct Vertex
{
    Vector3F Position;
    Vector3F Normal;
    Color4F Color;
    float UV[2];
};
