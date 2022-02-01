#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

class Window
{
public:
    Window(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual ~Window();
    virtual HRESULT Create();
    virtual HRESULT Prepare(int showCommand);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);
    static LRESULT CALLBACK Global(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    static void Main(InstanceHandle instance);

protected:
    InstanceHandle instance;
    WindowHandle window;
    std::wstring windowClassName;
    std::wstring windowTitle;

    DWORD windowExtensionStyle = WS_EX_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    int x = 100; // CW_USEDEFAULT;
    int y = 100; // CW_USEDEFAULT;
    int width = 640; // CW_USEDEFAULT;
    int height = 480; // CW_USEDEFAULT;

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
    virtual LRESULT Hit(LPARAM lParam);
    virtual LRESULT Safe(WPARAM wParam, LPARAM lParam);
};

class TransparentWindow : public BorderlessWindow
{
public:
    TransparentWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIFactory2> dxgiFactory;
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID2D1Factory2> d2dFactory;
    ComPtr<ID2D1Device1> d2dDevice;
    ComPtr<ID2D1DeviceContext> d2dDeviceContext;
    ComPtr<IDXGISurface2> dxgiSurface;
    ComPtr<ID2D1Bitmap1> d2dBitmap;
    ComPtr<IDCompositionDevice> dCompositionDevice;
    ComPtr<IDCompositionTarget> dCompositionTarget;
    ComPtr<IDCompositionVisual> dCompositionVisual;
};

class VisualizerWindow : public TransparentWindow
{
public:
    VisualizerWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT Paint();
    LRESULT Destroy();

private:
    bool isHovering = false;
};
