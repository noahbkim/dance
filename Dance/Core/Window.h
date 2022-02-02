#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

#include <vector>

class Window
{
public:
    Window(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual ~Window();
    virtual HRESULT Create();
    virtual HRESULT Prepare(int showCommand);
    virtual HRESULT Position(int x, int y, int width, int height, UINT flags);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);

protected:
    InstanceHandle instance;
    WindowHandle window;
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
    ComPtr<ID2D1DeviceContext> d2dDeviceContext;
    ComPtr<IDXGISurface2> dxgiSurface;
    ComPtr<ID2D1Bitmap1> d2dBitmap;
    ComPtr<IDCompositionDevice> dCompositionDevice;
    ComPtr<IDCompositionTarget> dCompositionTarget;
    ComPtr<IDCompositionVisual> dCompositionVisual;

    HRESULT CreateSurface();
    HRESULT ReleaseSurface();
    HRESULT CreateBitmap();
    HRESULT ReleaseBitmap();
    HRESULT CreateComposition();

    bool isResizingOrMoving = false;
    virtual LRESULT StartResizeMove();
    virtual LRESULT FinishResizeMove();

    RECT size;
};
