#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMCX
#define NOSERVICE
#define NOHELP

#include <windows.h>
#include <wrl.h>
#include <wingdi.h>
#include <d3d11_4.h>
#include <d2d1_2.h>

class Visualizer
{
public:
    struct Dependencies
    {
        // Window
        HINSTANCE Instance;
        HWND Window;

        // TransparentWindow
        Microsoft::WRL::ComPtr<ID3D11Device> D3dDevice;
        Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> DxgiSwapChain;
        Microsoft::WRL::ComPtr<ID2D1Device1> D2dDevice;
    };

    virtual ~Visualizer() {}

    // Allocation and deallocation of size-dependent resources
    virtual HRESULT Unsize() = 0;
    virtual HRESULT Resize(const RECT& size) = 0;

    // Runtime hooks
    virtual void Render() = 0;
    virtual void Update(double delta) = 0;
};
