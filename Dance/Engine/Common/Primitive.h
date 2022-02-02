#pragma once

struct Position3
{
	float x;
	float y;
	float z;
};

struct Color4
{
    Color4() : r(0), g(0), b(0), a(1.0) {}
    Color4(float r, float g, float b, float a = 1.0f)
        : r(r)
        , g(g)
        , b(b)
        , a(a) {}

    float r;
    float g;
    float b;
    float a;
};

struct PositionColorVertex
{
    Position3 position;
    Color4 color;
};

using PCVertex = PositionColorVertex;
using PositionColor = PositionColorVertex;
