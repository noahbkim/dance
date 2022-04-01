#include "TwoVisualizer.h"
#include "Macro.h"

namespace Dance::Two
{
	TwoVisualizer::TwoVisualizer(const Dependencies& dependencies)
		: d2dDevice(dependencies.D2dDevice)
		, dxgiSwapChain(dependencies.DxgiSwapChain)
	{
		OKE(this->d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			this->d2dDeviceContext.ReleaseAndGetAddressOf()));
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
		this->d2dDeviceContext->SetTarget(this->d2dBitmap.Get());

		return S_OK;
	}
}
