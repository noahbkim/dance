#pragma once

#include "Core/Window.h"
#include "Core/Runtime.h"
#include "Engine/Common/Primitive.h"
#include "Engine/Common/Buffer.h"

struct Bar
{
    float level;
    int direction;
};

class Visualizer : public TransparentWindow3D, public Runtime
{
public:
    Visualizer(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT Render();
    void Update(double delta);
    LRESULT Close();

private:
    bool isHovering = false;
    std::vector<Bar> levels;

    VertexBuffer<PositionColorVertex> vertices;

    ID3D11InputLayout* inputLayout;
    ID3D11PixelShader* pixelShader;
    ID3D11VertexShader* vertexShader;
    UINT numVerts;
    UINT stride;
    UINT offset;

    void RenderBorder();
};
