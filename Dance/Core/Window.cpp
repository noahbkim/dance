#include "Window.h"
#include "Common/Graphics.h"

Window::Window
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
) 
	: instance(instance)
	, window(nullptr)
	, windowClassName(windowClassName)
	, windowTitle(windowTitle)
{

}

Window::~Window()
{
	
}

HRESULT Window::Create()
{
	OK(this->Register());

	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
	this->window = WindowHandle(::CreateWindowExW(
		this->windowExtensionStyle,
		this->windowClassName.c_str(),
		this->windowTitle.c_str(),
		this->windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		this->instance,
		nullptr));

	if (!this->window)
	{
		TRACE("error creating window: " << ::GetLastError());
		return E_FAIL;
	}

	// Attach a reference to the wrapper using USERDATA
	// https://stackoverflow.com/questions/117792/best-method-for-storing-this-pointer-for-use-in-wndproc
	// https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-procedures
	BET(::SetWindowSubclass(this->window.get(), Window::Dispatch, 0, reinterpret_cast<DWORD_PTR>(this)));

	return S_OK;
}

HRESULT Window::Prepare(int showCommand)
{
	// Show and update
	::ShowWindow(this->window.get(), showCommand);
	BET(::UpdateWindow(this->window.get()));

	return S_OK;
}

HRESULT Window::Position(int x, int y, int width, int height, UINT flags)
{
	BET(::SetWindowPos(
		this->window.get(),
		nullptr,
		x,
		y,
		width,
		height,
		flags));
	return 0;
}

WNDCLASSEXW Window::Class()
{
	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = ::DefWindowProcW;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = this->instance;
	windowClass.hIcon = ::LoadIcon(this->instance, MAKEINTRESOURCE(IDI_DANCE));
	windowClass.hIconSm = ::LoadIcon(this->instance, MAKEINTRESOURCE(IDI_SMALL));
	windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	windowClass.lpszClassName = this->windowClassName.c_str();
	return windowClass;
}

HRESULT Window::Register()
{
	WNDCLASSEXW windowClass = this->Class();
	ATOM result = ::RegisterClassExW(&windowClass);
	return result ? S_OK : E_FAIL;
}

LRESULT CALLBACK Window::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProcW(windowHandle, message, wParam, lParam);
}

LRESULT CALLBACK Window::Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner)
{
	// https://stackoverflow.com/questions/35178779/wndproc-as-class-method
	Window* self = reinterpret_cast<Window*>(owner);
	return self->Message(windowHandle, message, wParam, lParam);
}

bool isMaximized(HWND window)
{
	WINDOWPLACEMENT placement{};
	return ::GetWindowPlacement(window, &placement) && placement.showCmd == SW_MAXIMIZE;
}

void adjustMaximizedClientRectangle(HWND window, RECT& rectangle)
{
	if (isMaximized(window))
	{
		HMONITOR monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (monitor == nullptr)
		{
			return;
		}

		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (!::GetMonitorInfoW(monitor, &monitorInfo))
		{
			return;
		}

		rectangle = monitorInfo.rcWork;
	}
}

BorderlessWindow::BorderlessWindow
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: Window(instance, windowClassName, windowTitle) 
{

}

HRESULT BorderlessWindow::Create()
{
	this->windowStyle = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	OK(Window::Create());

	// Redraw the frame so the safe area is recalculated
	BET(::SetWindowPos(this->window.get(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE));

	return S_OK;
}

LRESULT CALLBACK BorderlessWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCALCSIZE:
		return this->CalculateSize(wParam, lParam);
	case WM_NCHITTEST:
		return this->HitTest(lParam);
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

static constexpr LONG LEFT = 0b0001;
static constexpr LONG TOP = 0b0010;
static constexpr LONG RIGHT = 0b0100;
static constexpr LONG BOTTOM = 0b1000;
static constexpr LONG TOPLEFT = (TOP | LEFT);
static constexpr LONG TOPRIGHT = (TOP | RIGHT);
static constexpr LONG BOTTOMLEFT = (BOTTOM | LEFT);
static constexpr LONG BOTTOMRIGHT = (BOTTOM | RIGHT);

LRESULT BorderlessWindow::HitTest(LPARAM lParam)
{
	// https://github.com/melak47/BorderlessWindow/blob/master/BorderlessWindow/src/BorderlessWindow.cpp
	const POINT border
	{
		::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
		::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
	};

	RECT rectangle;
	BET(::GetWindowRect(this->window.get(), &rectangle));

	LONG x = GET_X_LPARAM(lParam);
	LONG y = GET_Y_LPARAM(lParam);

	LONG collision = (
		LEFT * (x < (rectangle.left + border.x)) |
		RIGHT * (x >= (rectangle.right - border.x)) |
		TOP * (y < (rectangle.top + border.y)) |
		BOTTOM * (y >= (rectangle.bottom - border.y)));
	
	switch (collision)
	{
	case LEFT: return HTLEFT;
	case RIGHT: return HTRIGHT;
	case TOP: return HTTOP;
	case BOTTOM: return HTBOTTOM;
	case TOPLEFT: return HTTOPLEFT;
	case TOPRIGHT: return HTTOPRIGHT;
	case BOTTOMLEFT: return HTBOTTOMLEFT;
	case BOTTOMRIGHT: return HTBOTTOMRIGHT;
	case 0: return HTCAPTION;
	default: return HTNOWHERE;
	}
}

LRESULT BorderlessWindow::CalculateSize(WPARAM wParam, LPARAM lParam)
{
	if (wParam == TRUE)
	{
		auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
		adjustMaximizedClientRectangle(this->window.get(), params.rgrc[0]);
	}
	return 0;
}

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

	::GetClientRect(this->window.get(), &this->size);

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

	// Create the Direct2D device context that is the actual render target
	// and exposes drawing commands
	OK(this->d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		this->d2dDeviceContext.ReleaseAndGetAddressOf()));

	OK(this->CreateSurface());
	OK(this->CreateBitmap());
	OK(this->CreateComposition());
}

HRESULT TransparentWindow::CreateSurface()
{
	// Retrieve the swap chain's back buffer
	OK(this->dxgiSwapChain->GetBuffer(
		0,
		__uuidof(this->dxgiSurface),
		reinterpret_cast<void**>(this->dxgiSurface.ReleaseAndGetAddressOf())));

	return S_OK;
}

HRESULT TransparentWindow::ReleaseSurface()
{
	this->dxgiSurface = nullptr;
	return S_OK;
}

HRESULT TransparentWindow::CreateBitmap()
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

HRESULT TransparentWindow::ReleaseBitmap()
{
	this->d2dBitmap = nullptr;
	return S_OK;
}

HRESULT TransparentWindow::CreateComposition()
{
	OK(::DCompositionCreateDevice(
		dxgiDevice.Get(),
		__uuidof(this->dCompositionDevice),
		reinterpret_cast<void**>(this->dCompositionDevice.ReleaseAndGetAddressOf())));
	OK(this->dCompositionDevice->CreateTargetForHwnd(
		this->window.get(),
		true, // Top most
		this->dCompositionTarget.ReleaseAndGetAddressOf()));
	OK(this->dCompositionDevice->CreateVisual(this->dCompositionVisual.ReleaseAndGetAddressOf()));

	OK(this->dCompositionVisual->SetContent(this->dxgiSwapChain.Get()));
	OK(this->dCompositionTarget->SetRoot(this->dCompositionVisual.Get()));
	OK(this->dCompositionDevice->Commit());

	return S_OK;
}

HRESULT TransparentWindow::Destroy()
{
	this->d2dDeviceContext->SetTarget(nullptr);
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
	case WM_NCCALCSIZE:
		return this->CalculateSize(wParam, lParam);
	case WM_NCHITTEST:
		return this->HitTest(lParam);
	case WM_ENTERSIZEMOVE:
		return this->StartResizeMove();
	case WM_EXITSIZEMOVE:
		return this->FinishResizeMove();
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

HRESULT TransparentWindow::Resize()
{
	// Unbind target and release bitmap and DXGI surface because they need to be recreated
	this->d2dDeviceContext->SetTarget(nullptr);
	OK(this->ReleaseBitmap());
	OK(this->ReleaseSurface());

	GetClientRect(this->window.get(), &this->size);

	// https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview?redirectedfrom=MSDN#resizing-a-dxgi-surface-render-target
	// Resize the swap chain
	HRESULT hr = this->dxgiSwapChain->ResizeBuffers(
		2,
		this->size.right - this->size.left,
		this->size.bottom - this->size.top,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0);
	OK(hr);

	// Recreate the DXGI surface and bind the bitmap that we releasesd prior
	OK(this->CreateSurface());
	OK(this->CreateBitmap());
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

TransparentWindow3D::TransparentWindow3D
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: TransparentWindow(instance, windowClassName, windowTitle)
{

}

HRESULT TransparentWindow3D::Create()
{
	OK(TransparentWindow::Create());

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

	OK(this->CreateRenderTarget());
	OK(this->CreateDepthStencil());
	return S_OK;
}

HRESULT TransparentWindow3D::CreateRenderTarget()
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

HRESULT TransparentWindow3D::ReleaseRenderTarget()
{
	this->d3dBackBufferView = nullptr;
	return S_OK;
}

HRESULT TransparentWindow3D::CreateDepthStencil()
{
	D3D11_TEXTURE2D_DESC descriptor{};
	descriptor.Width = this->size.right - this->size.left;
	descriptor.Height = this->size.bottom - this->size.top;
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

HRESULT TransparentWindow3D::ReleaseDepthStencil()
{
	this->d3dDepthStencilView = nullptr;
	this->d3dDepthTexture = nullptr;
	return S_OK;
}

HRESULT TransparentWindow3D::Resize()
{
	OK(this->ReleaseRenderTarget());
	OK(this->ReleaseDepthStencil());
	OK(TransparentWindow::Resize());
	OK(this->CreateRenderTarget());
	OK(this->CreateDepthStencil());
	return S_OK;
}
