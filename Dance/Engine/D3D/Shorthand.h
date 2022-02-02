#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

inline void SetViewport
(
    ID3D11DeviceContext* deviceContext,
    float x,
    float y,
    float width,
    float height
)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}
