#include "TransparentWindow.h"

namespace Dance::Application
{
#ifdef _DEBUG
	// Change some initialization flags to make live deebugging possible/easier.
	constexpr D2D1_FACTORY_OPTIONS D2D_FACTORY_CREATION_FLAGS{ D2D1_DEBUG_LEVEL_INFORMATION };
	constexpr UINT DXGI_FACTORY_CREATION_FLAGS = DXGI_CREATE_FACTORY_DEBUG;
	constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

	/// Enables the debug interface and reports on live objects at shutdown. Used to track down memory leaks.
	void dxgiDebug()
	{
		ComPtr<IDXGIDebug> dxgiDebug;
		DXGIGetDebugInterface1(
			0,
			__uuidof(dxgiDebug),
			reinterpret_cast<void**>(dxgiDebug.ReleaseAndGetAddressOf()));
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	}

#else
	// Default creation flags.
	constexpr D2D1_FACTORY_OPTIONS D2D_FACTORY_CREATION_FLAGS{};
	constexpr UINT DXGI_FACTORY_CREATION_FLAGS = 0;
	constexpr UINT D3D_DEVICE_CREATION_FLAGS = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	/// Not sure why this is the way it is or what feature level is necessary.
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
		swapChainDescription.Width = 1;
		swapChainDescription.Height = 1;

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

		OK(this->CreateComposition());

		return S_OK;
	}

	HRESULT TransparentWindow::CreateComposition()
	{
		OK(::DCompositionCreateDevice(
			this->dxgiDevice.Get(),
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
}
