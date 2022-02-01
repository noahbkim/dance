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
		this->x, 
		this->y,
		this->width,
		this->height,
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

LRESULT CALLBACK Window::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProcW(windowHandle, message, wParam, lParam);
}

WNDCLASSEXW Window::Class()
{
	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Window::Global;
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

LRESULT CALLBACK Window::Global(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProcW(windowHandle, message, wParam, lParam);
}

LRESULT CALLBACK Window::Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner)
{
	// https://stackoverflow.com/questions/35178779/wndproc-as-class-method
	Window* self = reinterpret_cast<Window*>(owner);
	return self->Message(windowHandle, message, wParam, lParam);
}

void Window::Main(InstanceHandle instance)
{
	HACCEL acceleratorTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDC_DANCE));
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(message.hwnd, acceleratorTable, &message))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			// Tick
		}
	}
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
		return this->Safe(wParam, lParam);
	case WM_NCHITTEST:
		return this->Hit(lParam);
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

static constexpr LONG LEFT = 0b0001;
static constexpr LONG TOP = 0b0010;
static constexpr LONG RIGHT = 0b0100;
static constexpr LONG BOTTOM = 0x1000;
static constexpr LONG TOPLEFT = (TOP | LEFT);
static constexpr LONG TOPRIGHT = (TOP | RIGHT);
static constexpr LONG BOTTOMLEFT = (BOTTOM | LEFT);
static constexpr LONG BOTTOMRIGHT = (BOTTOM | RIGHT);

LRESULT BorderlessWindow::Hit(LPARAM lParam)
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
		BF_LEFT * (x < (rectangle.left + border.x)) |
		BF_RIGHT * (x >= (rectangle.right - border.x)) |
		BF_TOP * (y < (rectangle.top + border.y)) |
		BF_BOTTOM * (y >= (rectangle.bottom - border.y)));
	
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

LRESULT BorderlessWindow::Safe(WPARAM wParam, LPARAM lParam)
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
		reinterpret_cast<void**>(this->dxgiFactory.GetAddressOf())));

	RECT rectangle{};
	::GetClientRect(this->window.get(), &rectangle);

	DXGI_SWAP_CHAIN_DESC1 swapChainDescription{};
	swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDescription.BufferCount = 2;
	swapChainDescription.SampleDesc.Count = 1;
	swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	swapChainDescription.Width = rectangle.right - rectangle.left;
	swapChainDescription.Height = rectangle.bottom - rectangle.top;

	// Create the composition swap chain with this description and create a pointer to the Direct3D device
	OK(this->dxgiFactory->CreateSwapChainForComposition(
		this->dxgiDevice.Get(),
		&swapChainDescription,
		nullptr,
		this->dxgiSwapChain.GetAddressOf()));

	// Create a single-threaded Direct2D factory with debugging information
	OK(::D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		D2D_FACTORY_CREATION_FLAGS,
		this->d2dFactory.GetAddressOf()));

	// Create the Direct2D device that links back to the Direct3D device
	OK(this->d2dFactory->CreateDevice(
		this->dxgiDevice.Get(),
		this->d2dDevice.GetAddressOf()));

	// Create the Direct2D device context that is the actual render target
	// and exposes drawing commands
	OK(this->d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		this->d2dDeviceContext.GetAddressOf()));

	// Retrieve the swap chain's back buffer
	OK(this->dxgiSwapChain->GetBuffer(
		0, // index
		__uuidof(this->dxgiSurface),
		reinterpret_cast<void**>(this->dxgiSurface.GetAddressOf())));

	// Create a Direct2D bitmap that points to the swap chain surface
	D2D1_BITMAP_PROPERTIES1 properties{};
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	OK(this->d2dDeviceContext->CreateBitmapFromDxgiSurface(
		this->dxgiSurface.Get(),
		properties,
		this->d2dBitmap.GetAddressOf()));

	OK(::DCompositionCreateDevice(
		dxgiDevice.Get(),
		__uuidof(this->dCompositionDevice),
		reinterpret_cast<void**>(this->dCompositionDevice.GetAddressOf())));
	OK(this->dCompositionDevice->CreateTargetForHwnd(
		this->window.get(),
		true, // Top most
		this->dCompositionTarget.GetAddressOf()));
	OK(this->dCompositionDevice->CreateVisual(this->dCompositionVisual.GetAddressOf()));

	OK(this->dCompositionVisual->SetContent(this->dxgiSwapChain.Get()));
	OK(this->dCompositionTarget->SetRoot(this->dCompositionVisual.Get()));
	OK(this->dCompositionDevice->Commit());
}

LRESULT CALLBACK TransparentWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return BorderlessWindow::Message(windowHandle, message, wParam, lParam);
}

VisualizerWindow::VisualizerWindow
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: TransparentWindow(instance, windowClassName, windowTitle)
{

}

LRESULT CALLBACK VisualizerWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCALCSIZE:
		return this->Safe(wParam, lParam);
	case WM_NCHITTEST:
		return this->Hit(lParam);
	case WM_PAINT:
		return this->Paint();
	case WM_DESTROY:
		return this->Destroy();
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

LRESULT VisualizerWindow::Paint()
{
	this->d2dDeviceContext->SetTarget(this->d2dBitmap.Get());
	// Draw something
	this->d2dDeviceContext->BeginDraw();
	this->d2dDeviceContext->Clear();
	ComPtr<ID2D1SolidColorBrush> brush;
	D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f,  // red
		0.55f,  // green
		0.34f,  // blue
		0.75f); // alpha
	this->d2dDeviceContext->CreateSolidColorBrush(brushColor, brush.GetAddressOf());
	D2D1_POINT_2F const ellipseCenter = D2D1::Point2F(150.0f,  // x
		150.0f); // y
	D2D1_ELLIPSE const ellipse = D2D1::Ellipse(ellipseCenter,
		100.0f,  // x radius
		100.0f); // y radius
	this->d2dDeviceContext->FillEllipse(ellipse,
		brush.Get());
	this->d2dDeviceContext->EndDraw();
	// Make the swap chain available to the composition engine

	this->dxgiSwapChain->Present(1,   // sync
		0); // flags

	return 0;
}

LRESULT VisualizerWindow::Destroy()
{
	::PostQuitMessage(0);
	return 0;
}
