#pragma once

#include "Framework.h"
#include "TransparentWindow.h"
#include "Common/Runtime.h"
#include "Visualizers/Bars.h"
#include "Visualizers/Cube.h"

class VisualizerWindow : public TransparentWindow, public Runtime
{
public:
    VisualizerWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

    virtual Visualizer::Dependencies Dependencies() const;

    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    void Render();
    void Update(double delta);

    LRESULT Close();

protected:
    bool isMouseHovering = false;
    bool isMouseTracking = false;

    virtual HRESULT Resize();

    LRESULT MouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT MouseHover(WPARAM wParam, LPARAM lParam);
    LRESULT MouseLeave(WPARAM wParam, LPARAM lParam);
    LRESULT RightButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT Command(WPARAM wParam, LPARAM lParam);

    std::unique_ptr<Visualizer> visualizer;
};
