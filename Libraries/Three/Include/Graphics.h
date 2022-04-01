#pragma once

#include "Three.h"

namespace Dance::Three::Graphics
{
#ifdef _DEBUG
    // Change some initialization flags to make live deebugging possible/easier.
    constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

    /// Enables the debug interface and reports on live objects at shutdown. Used to track down memory leaks.
    void dxgiDebug()
    {
        ComPtr<IDXGIDebug> dxgiDebug;
        DXGIGetDebugInterface1(
            0,
            __uuidof(dxgiDebug),
            reinterpret_cast<void**>(dxgiDebug.ReleaseAndGetAddressOf()));
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    }

#else
    // Default creation flags.
    constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#endif

    /// Not sure why this is the way it is or what feature level is necessary.
    constexpr D3D_FEATURE_LEVEL FEATURE_LEVELS[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
}
