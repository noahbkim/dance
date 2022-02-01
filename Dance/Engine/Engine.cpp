#include "engine.h"

void Engine::Setup(HWND hWnd, RECT rectangle)
{
	this->window.handle = hWnd;
	this->window.width = std::max(rectangle.right - rectangle.left, 1L);
	this->window.height = std::max(rectangle.bottom - rectangle.top, 1L);
	this->CreateDevice();
	this->CreateResources();
    this->visualizer = FourierVisualizer::Load();
}

void Engine::Teardown() 
{

}

void Engine::Tick()
{
    this->timer.Tick([&]() { this->Update(this->timer); });
    this->Render();
}

void Engine::Update(StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    elapsedTime;
}

void Engine::Render()
{
    if (this->timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    visualizer->Render(*this);

    Present();
}

void Engine::Clear()
{
    this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), DirectX::Colors::CornflowerBlue);
    this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(this->window.width), static_cast<float>(this->window.height));
    this->deviceContext->RSSetViewports(1, &viewport);
}

void Engine::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = this->swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);
    }
}

// Message handlers
void Engine::OnActivated()
{
    // TODO: Game is becoming active window.
    TRACE("current size: " << this->window.width << ", " << this->window.height);
}

void Engine::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Engine::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Engine::OnResuming()
{
    this->timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Engine::OnWindowSizeChanged(RECT rectangle)
{
    this->window.width = std::max(rectangle.right - rectangle.left, 1L);
    this->window.height = std::max(rectangle.bottom - rectangle.top, 1L);

    CreateResources();

    // TODO: Game window is being resized.
}


void Engine::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &this->featureLevel,       // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
    ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    ThrowIfFailed(device.As(&this->device));
    ThrowIfFailed(context.As(&this->deviceContext));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Engine::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    this->deviceContext->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
    this->renderTargetView.Reset();
    this->depthStencilView.Reset();
    this->deviceContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(this->window.width);
    const UINT backBufferHeight = static_cast<UINT>(this->window.height);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (this->swapChain)
    {
        HRESULT hr = this->swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

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
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        ThrowIfFailed(this->device.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            this->device.Get(),
            this->window.handle,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            this->swapChain.ReleaseAndGetAddressOf()
        ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        ThrowIfFailed(dxgiFactory->MakeWindowAssociation(this->window.handle, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(this->swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    ThrowIfFailed(this->device->CreateRenderTargetView(backBuffer.Get(), nullptr, this->renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    ThrowIfFailed(this->device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    ThrowIfFailed(this->device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, this->depthStencilView.ReleaseAndGetAddressOf()));
}

void Engine::OnDeviceLost()
{
    this->depthStencilView.Reset();
    this->renderTargetView.Reset();
    this->swapChain.Reset();
    this->deviceContext.Reset();
    this->device.Reset();

    CreateDevice();
    CreateResources();
}

RECT Engine::GetDefaultSize() const {
	return { 0, 0, 600, 480 };
}
