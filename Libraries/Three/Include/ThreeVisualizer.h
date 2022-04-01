#pragma once

#include "Visualizer.h"
#include "Pointer.h"

namespace Dance::Three
{
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
}
