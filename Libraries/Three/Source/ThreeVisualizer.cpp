#include "ThreeVisualizer.h"
#include "Macro.h"

namespace Dance::Three
{
	ThreeVisualizer::ThreeVisualizer(const Dependencies& dependencies)
		: dxgiSwapChain(dependencies.DxgiSwapChain)
		, d3dDevice(dependencies.D3dDevice)
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
		OKE(this->d3dDevice->CreateDepthStencilState(&descriptor, depthStencilState.ReleaseAndGetAddressOf()));
		this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
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
}
