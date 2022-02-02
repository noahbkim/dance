#pragma once

#include "Framework.h"

#ifdef _DEBUG
constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;
constexpr D2D1_FACTORY_OPTIONS D2D_FACTORY_CREATION_FLAGS = { D2D1_DEBUG_LEVEL_INFORMATION };
constexpr UINT DXGI_FACTORY_CREATION_FLAGS = DXGI_CREATE_FACTORY_DEBUG;
#else
constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
constexpr UINT D2D_FACTORY_CREATION_FLAGS = 0;
constexpr UINT DXGI_FACTORY_CREATION_FLAGS = 0;
#endif

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

void dxgiDebug()
{
    ComPtr<IDXGIDebug> dxgiDebug;
    DXGIGetDebugInterface1(
        0,
        __uuidof(dxgiDebug),
        reinterpret_cast<void**>(dxgiDebug.ReleaseAndGetAddressOf()));
    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
}