#include "dance.h"
#include "read.h"

Visualizer::Visualizer() noexcept {}

void Visualizer::Setup(HWND hWnd, RECT rectangle)
{
	this->window.handle = hWnd;
	this->window.width = std::max(rectangle.right - rectangle.left, 1L);
	this->window.height = std::max(rectangle.bottom - rectangle.top, 1L);
	this->CreateDevice();
	this->CreateResources();
    this->LoadShader("Shaders/Debug.hlsl");
}

void Visualizer::Teardown() 
{

}

void Visualizer::Tick()
{
    this->timer.Tick([&]() { Update(this->timer); });
    Render();
}

void Visualizer::Update(StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    elapsedTime;
}

void Visualizer::Render()
{
    if (this->timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.

    Present();
}

void Visualizer::Clear()
{
    ThrowIfFailed(this->graphics.commandAllocators[this->graphics.backBufferIndex]->Reset());
    ThrowIfFailed(this->graphics.commandList->Reset(
        this->graphics.commandAllocators[this->graphics.backBufferIndex].Get(),
        nullptr));

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        this->graphics.renderTargets[this->graphics.backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    this->graphics.commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
        this->graphics.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        static_cast<INT>(this->graphics.backBufferIndex), this->graphics.rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(this->graphics.dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    this->graphics.commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    this->graphics.commandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::CornflowerBlue, 0, nullptr);
    this->graphics.commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    D3D12_VIEWPORT viewport = { 
        0.0f, 
        0.0f, 
        static_cast<float>(this->window.width),
        static_cast<float>(this->window.height),
        D3D12_MIN_DEPTH,
        D3D12_MAX_DEPTH };
    this->graphics.commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect = {
        0,
        0,
        static_cast<LONG>(this->window.width),
        static_cast<LONG>(this->window.height) };
    this->graphics.commandList->RSSetScissorRects(1, &scissorRect);
}

void Visualizer::Present()
{
    // Transition the render target to the state that allows it to be presented to the display.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        this->graphics.renderTargets[this->graphics.backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    this->graphics.commandList->ResourceBarrier(1, &barrier);

    // Send the command list off to the GPU for processing.
    ThrowIfFailed(this->graphics.commandList->Close());
    this->graphics.commandQueue->ExecuteCommandLists(1, CommandListCast(this->graphics.commandList.GetAddressOf()));

    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = this->graphics.swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
        MoveToNextFrame();
    }
}

// Message handlers
void Visualizer::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Visualizer::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Visualizer::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Visualizer::OnResuming()
{
    this->timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Visualizer::OnWindowSizeChanged(RECT rectangle)
{
    this->window.width = std::max(rectangle.right - rectangle.left, 1L);
    this->window.height = std::max(rectangle.bottom - rectangle.top, 1L);

    CreateResources();

    // TODO: Game window is being resized.
}


void Visualizer::CreateDevice()
{
    DWORD dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
    }
#endif

    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(this->graphics.dxgiFactory.ReleaseAndGetAddressOf())));

    ComPtr<IDXGIAdapter1> adapter;
    GetAdapter(adapter.GetAddressOf());

    // Create the DX12 API device object.
    ThrowIfFailed(D3D12CreateDevice(
        adapter.Get(),
        this->graphics.featureLevel,
        IID_PPV_ARGS(this->graphics.d3dDevice.ReleaseAndGetAddressOf())
    ));

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(this->graphics.d3dDevice.As(&d3dInfoQueue)))
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] =
        {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(this->graphics.d3dDevice->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(this->graphics.commandQueue.ReleaseAndGetAddressOf())));

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = this->graphics.swapBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
    dsvDescriptorHeapDesc.NumDescriptors = 1;
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    ThrowIfFailed(this->graphics.d3dDevice->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc,
        IID_PPV_ARGS(this->graphics.rtvDescriptorHeap.ReleaseAndGetAddressOf())));
    ThrowIfFailed(this->graphics.d3dDevice->CreateDescriptorHeap(
        &dsvDescriptorHeapDesc,
        IID_PPV_ARGS(this->graphics.dsvDescriptorHeap.ReleaseAndGetAddressOf())));

    this->graphics.rtvDescriptorSize = this->graphics.d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < this->graphics.swapBufferCount; n++)
    {
        ThrowIfFailed(this->graphics.d3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(this->graphics.commandAllocators[n].ReleaseAndGetAddressOf())));
    }

    // Create a command list for recording graphics commands.
    ThrowIfFailed(this->graphics.d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        this->graphics.commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(this->graphics.commandList.ReleaseAndGetAddressOf())));
    ThrowIfFailed(this->graphics.commandList->Close());

    // Create a fence for tracking GPU execution progress.
    ThrowIfFailed(this->graphics.d3dDevice->CreateFence(
        this->graphics.fenceValues[this->graphics.backBufferIndex],
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(this->graphics.fence.ReleaseAndGetAddressOf())));
    this->graphics.fenceValues[this->graphics.backBufferIndex]++;

    this->graphics.fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (!this->graphics.fenceEvent.IsValid())
    {
        throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "CreateEventEx");
    }

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(this->graphics.d3dDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        TRACE("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Visualizer::CreateResources()
{
    // Wait until all previous GPU work is complete.
    this->WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < this->graphics.swapBufferCount; n++)
    {
        this->graphics.renderTargets[n].Reset();
        this->graphics.fenceValues[n] = this->graphics.fenceValues[this->graphics.backBufferIndex];
    }

    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
    const UINT backBufferWidth = static_cast<UINT>(this->window.width);
    const UINT backBufferHeight = static_cast<UINT>(this->window.height);

    // If the swap chain already exists, resize it, otherwise create one.
    if (this->graphics.swapChain)
    {
        HRESULT hr = this->graphics.swapChain->ResizeBuffers(this->graphics.swapBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }
    }
    else
    {
        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = this->graphics.swapBufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a swap chain for the window.
        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(this->graphics.dxgiFactory->CreateSwapChainForHwnd(
            this->graphics.commandQueue.Get(),
            this->window.handle,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            swapChain.GetAddressOf()
        ));

        ThrowIfFailed(swapChain.As(&this->graphics.swapChain));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut
        ThrowIfFailed(this->graphics.dxgiFactory->MakeWindowAssociation(this->window.handle, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < this->graphics.swapBufferCount; n++)
    {
        ThrowIfFailed(this->graphics.swapChain->GetBuffer(n, IID_PPV_ARGS(this->graphics.renderTargets[n].GetAddressOf())));

        wchar_t name[25] = {};
        swprintf_s(name, L"Render target %u", n);
        this->graphics.renderTargets[n]->SetName(name);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
            this->graphics.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(n), this->graphics.rtvDescriptorSize);
        this->graphics.d3dDevice->CreateRenderTargetView(this->graphics.renderTargets[n].Get(), nullptr, rtvDescriptor);
    }

    // Reset the index to the current back buffer.
    this->graphics.backBufferIndex = this->graphics.swapChain->GetCurrentBackBufferIndex();

    // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
    // on this surface.
    CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthBufferFormat,
        backBufferWidth,
        backBufferHeight,
        1, // This depth stencil view has only one texture.
        1  // Use a single mipmap level.
    );
    depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = depthBufferFormat;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(this->graphics.d3dDevice->CreateCommittedResource(
        &depthHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(this->graphics.depthStencil.ReleaseAndGetAddressOf())
    ));

    this->graphics.depthStencil->SetName(L"Depth stencil");

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    this->graphics.d3dDevice->CreateDepthStencilView(this->graphics.depthStencil.Get(), &dsvDesc, this->graphics.dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // TODO: Initialize windows-size dependent objects here.
}

void Visualizer::LoadShader(LPWSTR path)
{
    // Create the pipeline state, which includes compiling and loading shaders.
    {
        byte* data;
        uint32_t size;
        ReadDataFromFile(path, &data, &size);

        D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.PS = { data, size };
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = this->graphics.renderTargets[0]->GetDesc().Format;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    // CW front; cull back
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc = DefaultSampleDesc();

        auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
        streamDesc.pPipelineStateSubobjectStream = &psoStream;
        streamDesc.SizeInBytes = sizeof(psoStream);

        ThrowIfFailed(this->graphics.d3dDevice->CreateGraphicsPipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    m_model.LoadFromFile(c_meshFilename);
    m_model.UploadGpuResources(m_device.Get(), m_commandQueue.Get(), m_commandAllocators[m_frameIndex].Get(), m_commandList.Get());

#ifdef _DEBUG
    // Mesh shader file expects a certain vertex layout; assert our mesh conforms to that layout.
    const D3D12_INPUT_ELEMENT_DESC c_elementDescs[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
    };

    for (auto& mesh : m_model)
    {
        assert(mesh.LayoutDesc.NumElements == 2);

        for (uint32_t i = 0; i < _countof(c_elementDescs); ++i)
            assert(std::memcmp(&mesh.LayoutElems[i], &c_elementDescs[i], sizeof(D3D12_INPUT_ELEMENT_DESC)) == 0);
    }
#endif

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForGpu();
    }
}

void Visualizer::WaitForGpu() noexcept
{
    if (this->graphics.commandQueue && this->graphics.fence && this->graphics.fenceEvent.IsValid())
    {
        // Schedule a Signal command in the GPU queue.
        UINT64 fenceValue = this->graphics.fenceValues[this->graphics.backBufferIndex];
        if (SUCCEEDED(this->graphics.commandQueue->Signal(this->graphics.fence.Get(), fenceValue)))
        {
            // Wait until the Signal has been processed.
            if (SUCCEEDED(this->graphics.fence->SetEventOnCompletion(fenceValue, this->graphics.fenceEvent.Get())))
            {
                WaitForSingleObjectEx(this->graphics.fenceEvent.Get(), INFINITE, FALSE);

                // Increment the fence value for the current frame.
                this->graphics.fenceValues[this->graphics.backBufferIndex]++;
            }
        }
    }
}

void Visualizer::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = this->graphics.fenceValues[this->graphics.backBufferIndex];
    ThrowIfFailed(this->graphics.commandQueue->Signal(this->graphics.fence.Get(), currentFenceValue));

    // Update the back buffer index.
    this->graphics.backBufferIndex = this->graphics.swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (this->graphics.fence->GetCompletedValue() < this->graphics.fenceValues[this->graphics.backBufferIndex])
    {
        ThrowIfFailed(this->graphics.fence->SetEventOnCompletion(
            this->graphics.fenceValues[this->graphics.backBufferIndex],
            this->graphics.fenceEvent.Get()));
        WaitForSingleObjectEx(this->graphics.fenceEvent.Get(), INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    this->graphics.fenceValues[this->graphics.backBufferIndex] = currentFenceValue + 1;
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, try WARP. Otherwise throw an exception.
void Visualizer::GetAdapter(IDXGIAdapter1** ppAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != this->graphics.dxgiFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        ThrowIfFailed(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), this->graphics.featureLevel, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

#if !defined(NDEBUG)
    if (!adapter)
    {
        if (FAILED(this->graphics.dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
        {
            throw std::runtime_error("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }
    }
#endif

    if (!adapter)
    {
        throw std::runtime_error("No Direct3D 12 device found");
    }

    *ppAdapter = adapter.Detach();
}

void Visualizer::OnDeviceLost()
{
    // TODO: Perform Direct3D resource cleanup.

    for (UINT n = 0; n < this->graphics.swapBufferCount; n++)
    {
        this->graphics.commandAllocators[n].Reset();
        this->graphics.renderTargets[n].Reset();
    }

    this->graphics.depthStencil.Reset();
    this->graphics.fence.Reset();
    this->graphics.commandList.Reset();
    this->graphics.swapChain.Reset();
    this->graphics.rtvDescriptorHeap.Reset();
    this->graphics.dsvDescriptorHeap.Reset();
    this->graphics.commandQueue.Reset();
    this->graphics.d3dDevice.Reset();
    this->graphics.dxgiFactory.Reset();

    CreateDevice();
    CreateResources();
}

RECT Visualizer::GetDefaultSize() const {
	return { 0, 0, 600, 480 };
}
