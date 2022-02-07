#pragma once

#include "Framework.h"
#include "BorderlessWindow.h"

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
