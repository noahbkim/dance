#pragma once

#include "Framework.h"
#include "Common/Pointer.h"
#include "Audio/Audio.h"
#include "Audio/Device.h"
#include "Audio/AudioAnalyzer.h"
 
#include <vector>

class Visualizer
{
public:
    struct Dependencies
    {
        // Window
        HINSTANCE Instance;
        HWND Window;

        // TransparentWindow
        ComPtr<ID3D11Device> D3dDevice;
        ComPtr<IDXGIDevice> DxgiDevice;
        ComPtr<IDXGISwapChain1> DxgiSwapChain;
        ComPtr<ID2D1Device1> D2dDevice;
    };

    virtual ~Visualizer();

    // Allocation and deallocation of size-dependent resources
    virtual HRESULT Unsize() = 0;
    virtual HRESULT Resize(const RECT& size) = 0;

    // Runtime hooks
    virtual void Render() = 0;
    virtual void Update(double delta) = 0;
};

class AudioVisualizer : public virtual Visualizer
{
public:
    AudioVisualizer(const Dependencies& dependencies);
    virtual ~AudioVisualizer();

    virtual void Update(double delta);

protected:
    AudioAnalyzer analyzer;
};

class TwoVisualizer : public virtual Visualizer
{
public:
    TwoVisualizer(const Dependencies& dependencies);

    virtual HRESULT Unsize();
    virtual HRESULT Resize(const RECT& size);

protected:
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID2D1Device1> d2dDevice;
    ComPtr<IDXGISurface2> dxgiSurface;
    ComPtr<ID2D1Bitmap1> d2dBitmap;
    ComPtr<ID2D1DeviceContext> d2dDeviceContext;

    HRESULT CreateSurface();
    HRESULT ReleaseSurface();
    HRESULT CreateBitmap();
    HRESULT ReleaseBitmap();
};

class ThreeVisualizer : public virtual Visualizer
{
public:
    ThreeVisualizer(const Dependencies& dependencies);

    virtual HRESULT Unsize();
    virtual HRESULT Resize(const RECT& size);

protected:
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;
    ComPtr<ID3D11RenderTargetView> d3dBackBufferView;
    ComPtr<ID3D11DepthStencilView> d3dDepthStencilView;
    ComPtr<ID3D11Texture2D> d3dDepthTexture;
    ComPtr<ID3D11SamplerState> d3dSamplerState;

    HRESULT CreateRenderTarget();
    HRESULT ReleaseRenderTarget();
    HRESULT CreateDepthStencil(const RECT& size);
    HRESULT ReleaseDepthStencil();
};
