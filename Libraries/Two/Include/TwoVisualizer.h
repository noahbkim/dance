#pragma once

#include "Visualizer.h"
#include "Pointer.h"

namespace Dance::Two
{
    class TwoVisualizer : public virtual Dance::API::Visualizer
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
}