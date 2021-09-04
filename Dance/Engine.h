#pragma once

#include "framework.h"
#include "steptimer.h"
#include "visualizer.h"

using Microsoft::WRL::ComPtr;

class Engine {
public:
    void Setup(HWND hWnd, RECT rectangle);
    void Teardown();

    void Tick();

    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(RECT rectangle);

    RECT GetDefaultSize() const;

private:
    void Update(StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    struct {
        HWND handle = 0;
        size_t width = 1;
        size_t height = 1;
    } window;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;
    ComPtr<ID3D11Device1> device;
    ComPtr<ID3D11DeviceContext1> deviceContext;

    ComPtr<IDXGISwapChain1> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11DepthStencilView> depthStencilView;

    std::unique_ptr<Visualizer> visualizer;

    StepTimer timer;
};
