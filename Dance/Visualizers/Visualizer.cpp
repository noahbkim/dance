#include "Visualizer.h"

Visualizer::~Visualizer()
{

}

void Visualizer::Border(ComPtr<ID2D1DeviceContext> context, const RECT& size)
{
	ComPtr<ID2D1SolidColorBrush> brush;
	D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.5f);
	context->CreateSolidColorBrush(color, brush.GetAddressOf());

	static const FLOAT THICKNESS = 2.0f;
	const FLOAT w = size.right - size.left;
	const FLOAT h = size.bottom - size.top;

	D2D1_RECT_F stroke{ 0, 0, w, THICKNESS };
	context->FillRectangle(stroke, brush.Get());
	stroke = { 0, h - THICKNESS, w, h };
	context->FillRectangle(stroke, brush.Get());
	stroke = { 0, THICKNESS, THICKNESS, h - THICKNESS };
	context->FillRectangle(stroke, brush.Get());
	stroke = { w - THICKNESS, THICKNESS, w, h - THICKNESS };
	context->FillRectangle(stroke, brush.Get());
}

HRESULT AudioVisualizer::Create(const Visualizer::Dependencies& dependencies)
{
    ComPtr<IMMDevice> device = getDefaultDevice();
    TRACE("capturing " << getDeviceFriendlyName(device.Get()));
    this->analyzer = AudioAnalyzer(device, ONE_SECOND / 10);
    this->spectrum.resize(this->analyzer.Window());
    this->analyzer.Sink(reinterpret_cast<fftwf_complex*>(this->spectrum.data()));
    this->analyzer.Enable();
	return S_OK;
}

void AudioVisualizer::Update(double delta)
{
    if (this->analyzer.Listen())
    {
        this->analyzer.Analyze();
    }
}

HRESULT AudioVisualizer::Destroy()
{
    this->analyzer.Disable();
	return S_OK;
}

HRESULT TwoVisualizer::Create(const Dependencies& dependencies)
{
    this->d2dDevice = dependencies.D2dDevice;
	this->dxgiSwapChain = dependencies.DxgiSwapChain;

    OK(this->d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        this->d2dDeviceContext.ReleaseAndGetAddressOf()));
}

HRESULT TwoVisualizer::Destroy()
{
    this->d2dDeviceContext->SetTarget(nullptr);
	return S_OK;
}


HRESULT TwoVisualizer::CreateSurface()
{
	// Retrieve the swap chain's back buffer
	OK(this->dxgiSwapChain->GetBuffer(
		0,
		__uuidof(this->dxgiSurface),
		reinterpret_cast<void**>(this->dxgiSurface.ReleaseAndGetAddressOf())));

	return S_OK;
}

HRESULT TwoVisualizer::ReleaseSurface()
{
	this->dxgiSurface = nullptr;
	return S_OK;
}

HRESULT TwoVisualizer::CreateBitmap()
{
	// Create a Direct2D bitmap that points to the swap chain surface
	D2D1_BITMAP_PROPERTIES1 properties{};
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	OK(this->d2dDeviceContext->CreateBitmapFromDxgiSurface(
		this->dxgiSurface.Get(),
		properties,
		this->d2dBitmap.ReleaseAndGetAddressOf()));

	return S_OK;
}

HRESULT TwoVisualizer::ReleaseBitmap()
{
	this->d2dBitmap = nullptr;
	return S_OK;
}

HRESULT TwoVisualizer::Unsize()
{
	// Unbind target and release bitmap and DXGI surface because they need to be recreated
	this->d2dDeviceContext->SetTarget(nullptr);
	OK(this->ReleaseBitmap());
	OK(this->ReleaseSurface());

	return S_OK;
}

HRESULT TwoVisualizer::Resize(const RECT& size)
{
	// Recreate the DXGI surface and bind the bitmap that we releasesd prior
	OK(this->CreateSurface());
	OK(this->CreateBitmap());

	return S_OK;
}

HRESULT ThreeVisualizer::Create(const Dependencies& dependencies)
{
	// https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview
	this->d3dDevice->GetImmediateContext(this->d3dDeviceContext.ReleaseAndGetAddressOf());

	// Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC descriptor{};
	descriptor.DepthEnable = true;
	descriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descriptor.DepthFunc = D3D11_COMPARISON_LESS;
	descriptor.StencilEnable = false;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	OK(this->d3dDevice->CreateDepthStencilState(&descriptor, depthStencilState.ReleaseAndGetAddressOf()));
	this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);

	return S_OK;
}


HRESULT ThreeVisualizer::CreateRenderTarget()
{
	// Build the backbuffer
	ComPtr<ID3D11Texture2D> backBufferTexture;
	OK(this->dxgiSwapChain->GetBuffer(
		0,
		__uuidof(backBufferTexture),
		reinterpret_cast<void**>(backBufferTexture.ReleaseAndGetAddressOf())));

	// Set it up as a render target
	OK(this->d3dDevice->CreateRenderTargetView(
		backBufferTexture.Get(),
		nullptr,
		this->d3dBackBufferView.ReleaseAndGetAddressOf()));

	return S_OK;
}

HRESULT ThreeVisualizer::ReleaseRenderTarget()
{
	this->d3dBackBufferView = nullptr;
	return S_OK;
}

HRESULT ThreeVisualizer::CreateDepthStencil(const RECT& size)
{
	D3D11_TEXTURE2D_DESC descriptor{};
	descriptor.Width = size.right - size.left;
	descriptor.Height = size.bottom - size.top;
	descriptor.MipLevels = 1;
	descriptor.ArraySize = 1;
	descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descriptor.SampleDesc.Count = 1;
	descriptor.SampleDesc.Quality = 0;
	descriptor.Usage = D3D11_USAGE_DEFAULT;
	descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descriptor.CPUAccessFlags = 0;
	descriptor.MiscFlags = 0;
	OK(this->d3dDevice->CreateTexture2D(&descriptor, nullptr, this->d3dDepthTexture.ReleaseAndGetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDescriptor{};
	viewDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	viewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDescriptor.Texture2D.MipSlice = 0;
	OK(this->d3dDevice->CreateDepthStencilView(
		this->d3dDepthTexture.Get(),
		&viewDescriptor,
		this->d3dDepthStencilView.ReleaseAndGetAddressOf()));

	return S_OK;
}

HRESULT ThreeVisualizer::ReleaseDepthStencil()
{
	this->d3dDepthStencilView = nullptr;
	this->d3dDepthTexture = nullptr;
	return S_OK;
}

HRESULT ThreeVisualizer::Unsize()
{
	OK(this->ReleaseRenderTarget());
	OK(this->ReleaseDepthStencil());
	return S_OK;
}

HRESULT ThreeVisualizer::Resize(const RECT& size)
{
	OK(this->CreateRenderTarget());
	OK(this->CreateDepthStencil(size));
	return S_OK;
}
