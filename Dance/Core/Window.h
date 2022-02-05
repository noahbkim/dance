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
    virtual HRESULT Destroy();

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

class TransparentWindow2D : public TransparentWindow
{
public:
    TransparentWindow2D(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual HRESULT Destroy();

protected:
    ComPtr<IDXGISurface2> dxgiSurface;
    ComPtr<ID2D1Bitmap1> d2dBitmap;
    ComPtr<ID2D1DeviceContext> d2dDeviceContext;

    HRESULT CreateSurface();
    HRESULT ReleaseSurface();
    HRESULT CreateBitmap();
    HRESULT ReleaseBitmap();
    virtual HRESULT Resize();

    bool is2DEnabled = false;
};

class TransparentWindow3D : public TransparentWindow2D
{
public:
    TransparentWindow3D(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();

protected:
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;
    ComPtr<ID3D11RenderTargetView> d3dBackBufferView;
    ComPtr<ID3D11RenderTargetView> d3dRenderTargetView;
    ComPtr<ID3D11DepthStencilView> d3dDepthStencilView;
    ComPtr<ID3D11Texture2D> d3dDepthTexture;
    ComPtr<ID3D11SamplerState> d3dSamplerState;

    HRESULT CreateRenderTarget();
    HRESULT ReleaseRenderTarget();
    HRESULT CreateDepthStencil();
    HRESULT ReleaseDepthStencil();
    virtual HRESULT Resize();

    bool is3DEnabled = false;
};
