#include "TransparentWindow.h"
#include "Graphics/Graphics.h"

TransparentWindow::TransparentWindow
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: BorderlessWindow(instance, windowClassName, windowTitle)
{

}

HRESULT TransparentWindow::Create()
{
	this->windowExtensionStyle = WS_EX_NOREDIRECTIONBITMAP;
	BorderlessWindow::Create();

	// https://docs.microsoft.com/en-us/archive/msdn-magazine/2014/june/windows-with-c-high-performance-window-layering-using-the-windows-composition-engine
	OK(::D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D_DEVICE_CREATION_FLAGS,
		FEATURE_LEVELS,
		static_cast<UINT>(std::size(FEATURE_LEVELS)),
		D3D11_SDK_VERSION,
		&this->d3dDevice,
		nullptr,
		nullptr));

	OK(this->d3dDevice.As(&dxgiDevice));
	OK(::CreateDXGIFactory2(
		DXGI_FACTORY_CREATION_FLAGS,
		__uuidof(this->dxgiFactory),
		reinterpret_cast<void**>(this->dxgiFactory.ReleaseAndGetAddressOf())));

	::GetClientRect(this->window, &this->size);

	DXGI_SWAP_CHAIN_DESC1 swapChainDescription{};
	swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDescription.BufferCount = 2;
	swapChainDescription.SampleDesc.Count = 1;
	swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	swapChainDescription.Width = this->size.right - this->size.left;
	swapChainDescription.Height = this->size.bottom - this->size.top;

	// Create the composition swap chain with this description and create a pointer to the Direct3D device
	OK(this->dxgiFactory->CreateSwapChainForComposition(
		this->dxgiDevice.Get(),
		&swapChainDescription,
		nullptr,
		this->dxgiSwapChain.ReleaseAndGetAddressOf()));

	// Create a single-threaded Direct2D factory with debugging information
	OK(::D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		D2D_FACTORY_CREATION_FLAGS,
		this->d2dFactory.ReleaseAndGetAddressOf()));

	// Create the Direct2D device that links back to the Direct3D device
	OK(this->d2dFactory->CreateDevice(
		this->dxgiDevice.Get(),
		this->d2dDevice.ReleaseAndGetAddressOf()));

	OK(this->CreateComposition());

	return S_OK;
}

HRESULT TransparentWindow::CreateComposition()
{
	OK(::DCompositionCreateDevice(
		dxgiDevice.Get(),
		__uuidof(this->dCompositionDevice),
		reinterpret_cast<void**>(this->dCompositionDevice.ReleaseAndGetAddressOf())));
	OK(this->dCompositionDevice->CreateTargetForHwnd(
		this->window,
		true, // Top most
		this->dCompositionTarget.ReleaseAndGetAddressOf()));
	OK(this->dCompositionDevice->CreateVisual(this->dCompositionVisual.ReleaseAndGetAddressOf()));

	OK(this->dCompositionVisual->SetContent(this->dxgiSwapChain.Get()));
	OK(this->dCompositionTarget->SetRoot(this->dCompositionVisual.Get()));
	OK(this->dCompositionDevice->Commit());

	return S_OK;
}

HRESULT TransparentWindow::Position(int x, int y, int width, int height, UINT flags)
{
	OK(BorderlessWindow::Position(x, y, width, height, flags));
	OK(this->Resize());
	return S_OK;
}

LRESULT CALLBACK TransparentWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ENTERSIZEMOVE:
		return this->StartResizeMove();
	case WM_EXITSIZEMOVE:
		return this->FinishResizeMove();
	default:
        return BorderlessWindow::Message(windowHandle, message, wParam, lParam);
	}
}

HRESULT TransparentWindow::Resize()
{
	::GetClientRect(this->window, &this->size);

	// https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview?redirectedfrom=MSDN#resizing-a-dxgi-surface-render-target
	// Resize the swap chain
	OK(this->dxgiSwapChain->ResizeBuffers(
		2,
		this->size.right - this->size.left,
		this->size.bottom - this->size.top,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0));

	return S_OK;
}

LRESULT TransparentWindow::StartResizeMove()
{
	this->isResizingOrMoving = true;
	return 0;
}

LRESULT TransparentWindow::FinishResizeMove()
{
	this->isResizingOrMoving = false;
	OK(this->Resize());
	return 0;

	/*
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		// If the device was removed for any reason, a new device and swap chain will need to be created.
		OnDeviceLost();

		// Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method
		// and correctly set up the new device.
		return;
	}
	*/
}
