#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

#include "Core/Window.h"
#include "Core/Runtime.h"
#include "Core/Audio.h"
#include "Common/Primitive.h"
#include "Common/Buffer.h"
#include "Common/Shader.h"
#include "Common/Camera.h"
#include "Visualizers/Bars.h"
#include "Visualizers/Cube.h"

class Window
{
public:
    Window(HINSTANCE instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual ~Window();
    virtual HRESULT Create();
    virtual HRESULT Prepare(int showCommand);
    virtual HRESULT Position(int x, int y, int width, int height, UINT flags);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT Destroy();

    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);

protected:
    HINSTANCE instance;
    HWND window;
    std::wstring windowClassName;
    std::wstring windowTitle;
    DWORD windowExtensionStyle = WS_EX_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;

    virtual WNDCLASSEXW Class();
    virtual HRESULT Register();
};

class BorderlessWindow : public Window
{
public:
    BorderlessWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    virtual LRESULT HitTest(LPARAM lParam);
    virtual LRESULT CalculateSize(WPARAM wParam, LPARAM lParam);
};

class TransparentWindow : public BorderlessWindow
{
public:
    TransparentWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual HRESULT Position(int x, int y, int width, int height, UINT flags);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIFactory2> dxgiFactory;
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID2D1Factory2> d2dFactory;
    ComPtr<ID2D1Device1> d2dDevice;
    ComPtr<IDCompositionDevice> dCompositionDevice;
    ComPtr<IDCompositionTarget> dCompositionTarget;
    ComPtr<IDCompositionVisual> dCompositionVisual;

    HRESULT CreateComposition();
    virtual HRESULT Resize();

    bool isResizingOrMoving = false;
    virtual LRESULT StartResizeMove();
    virtual LRESULT FinishResizeMove();

    RECT size{};
};

class VisualizerWindow : public TransparentWindow, public Runtime
{
public:
    VisualizerWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

    virtual Visualizer::Dependencies Dependencies() const;

    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    void Render();
    void Update(double delta);

    LRESULT MouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT MouseHover(WPARAM wParam, LPARAM lParam);
    LRESULT MouseLeave(WPARAM wParam, LPARAM lParam);
    LRESULT RightButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT Command(WPARAM wParam, LPARAM lParam);

    LRESULT Close();

protected:
    virtual HRESULT Resize();

private:
    bool isMouseHovering = false;
    bool isMouseTracking = false;

    std::unique_ptr<BarsVisualizer> visualizer;

    void RenderBorder();
};
