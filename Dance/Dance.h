#pragma once

#include "framework.h"
#include "steptimer.h"

using Microsoft::WRL::ComPtr;

class Visualizer {
public:
    Visualizer() noexcept;
    ~Visualizer() = default;

    // Initialization and management
    void Setup(HWND hWnd, RECT rectangle);
    void Teardown();

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(RECT rectangle);

    // Default size
    RECT GetDefaultSize() const;

private:
    void Update(StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void LoadShader(LPWSTR path);

    void WaitForGpu() noexcept;
    void MoveToNextFrame();
    void GetAdapter(IDXGIAdapter1** ppAdapter);

    void OnDeviceLost();

    struct {
        HWND handle;
        size_t width;
        size_t height;
    } window;

    struct {
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        static const UINT swapBufferCount = 2;

        UINT backBufferIndex = 0;
        UINT rtvDescriptorSize = 0;

        ComPtr<ID3D12Device> d3dDevice;
        ComPtr<IDXGIFactory4> dxgiFactory;
        ComPtr<ID3D12CommandQueue> commandQueue;
        ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
        ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
        ComPtr<ID3D12CommandAllocator> commandAllocators[swapBufferCount];
        ComPtr<ID3D12GraphicsCommandList> commandList;
        ComPtr<ID3D12Fence> fence;
        UINT64 fenceValues[swapBufferCount] {};
        Microsoft::WRL::Wrappers::Event fenceEvent;

        // Shader
        ComPtr<ID3D12PipelineState> pipelineState;
        ComPtr<ID3D12Resource> constantBuffer;

        ComPtr<IDXGISwapChain3> swapChain;
        ComPtr<ID3D12Resource> renderTargets[swapBufferCount];
        ComPtr<ID3D12Resource> depthStencil;
    } graphics;

    StepTimer timer;
};
