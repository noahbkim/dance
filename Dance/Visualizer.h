#pragma once

#include "Core/Window.h"
#include "Core/Runtime.h"

struct Bar
{
    float level;
    int direction;
};

class Visualizer : public TransparentWindow, public Runtime
{
public:
    Visualizer(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT Render();
    void Update(double delta);
    LRESULT Close();

private:
    bool isHovering = false;
    std::vector<Bar> levels;

    void RenderBorder(ComPtr<ID2D1DeviceContext> context);
};
