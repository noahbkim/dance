#include "Window.h"
#include "Common/Graphics.h"
#include "Common/Camera.h"
#include "Mathematics.h"
#include "Visualizers/Bars.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

Window::Window
(
	HINSTANCE instance,
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
	this->window = ::CreateWindowExW(
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
		nullptr);

	if (!this->window)
	{
		TRACE("error creating window: " << ::GetLastError());
		return E_FAIL;
	}

	// Attach a reference to the wrapper using USERDATA
	// https://stackoverflow.com/questions/117792/best-method-for-storing-this-pointer-for-use-in-wndproc
	// https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-procedures
	BET(::SetWindowSubclass(this->window, Window::Dispatch, 0, reinterpret_cast<DWORD_PTR>(this)));

	return S_OK;
}

HRESULT Window::Prepare(int showCommand)
{
	// Show and update
	::ShowWindow(this->window, showCommand);
	BET(::UpdateWindow(this->window));

	return S_OK;
}

HRESULT Window::Position(int x, int y, int width, int height, UINT flags)
{
	BET(::SetWindowPos(
		this->window,
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

HRESULT Window::Destroy()
{
	::DestroyWindow(this->window);
	return S_OK;
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
	BET(::SetWindowPos(this->window, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE));

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
	BET(::GetWindowRect(this->window, &rectangle));

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
		NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
		adjustMaximizedClientRectangle(this->window, params->rgrc[0]);
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
	::GetClientRect(this->window, &this->size);

	// https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-and-direct3d-interoperation-overview?redirectedfrom=MSDN#resizing-a-dxgi-surface-render-target
	// Resize the swap chain
	HRESULT hr = this->dxgiSwapChain->ResizeBuffers(
		2,
		this->size.right - this->size.left,
		this->size.bottom - this->size.top,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0);
	OK(hr);

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

static const int MENU_EXIT = 42;

VisualizerWindow::VisualizerWindow
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: TransparentWindow(instance, windowClassName, windowTitle)
	, Runtime()
{

}

Visualizer::Dependencies VisualizerWindow::Dependencies() const
{
    return {
        this->instance,
        this->window,
        this->d3dDevice,
        this->dxgiDevice,
        this->dxgiSwapChain,
        this->d2dDevice
    };
}

HRESULT VisualizerWindow::Create()
{
	TransparentWindow::Create();
	this->visualizer = std::make_unique<BarsVisualizer>();
	this->visualizer->Create(this->Dependencies());
	return S_OK;
}

HRESULT VisualizerWindow::Resize()
{
	OK(this->visualizer->Unsize());
	OK(TransparentWindow::Resize());
	OK(this->visualizer->Resize(this->size));
	return S_OK;
}

LRESULT CALLBACK VisualizerWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
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
	case WM_MOUSEHOVER:
	case WM_NCMOUSEHOVER:
		return this->MouseHover(wParam, lParam);
	case WM_MOUSELEAVE:
	case WM_NCMOUSELEAVE:
		return this->MouseLeave(wParam, lParam);
	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		return this->MouseMove(wParam, lParam);
	case WM_RBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		return this->RightButtonDown(wParam, lParam);
	case WM_COMMAND:
		return this->Command(wParam, lParam);
	case WM_PAINT:
		this->Render();
		return 0;
	case WM_DESTROY:
		return this->Close();
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

void VisualizerWindow::Render()
{
	this->visualizer->Render();
}

void VisualizerWindow::Update(double delta)
{
	this->visualizer->Update(delta);
}

LRESULT VisualizerWindow::MouseMove(WPARAM wParam, LPARAM lParam)
{
	this->isMouseHovering = true;

	static const MARGINS shadow_state{ 1, 1, 1, 1 };
	OK(::DwmExtendFrameIntoClientArea(this->window, &shadow_state));

	if (!this->isMouseTracking)
	{
		TRACKMOUSEEVENT tracking{};
		tracking.cbSize = sizeof(tracking);
		tracking.dwFlags = TME_NONCLIENT | TME_LEAVE;
		tracking.hwndTrack = this->window;
		tracking.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tracking);
		this->isMouseTracking = true;
	}
	return 0;
}

LRESULT VisualizerWindow::MouseHover(WPARAM wParam, LPARAM lParam)
{
	this->isMouseHovering = true;

	static const MARGINS shadow_state{ 1, 1, 1, 1 };
	OK(::DwmExtendFrameIntoClientArea(this->window, &shadow_state));

	return 0;
}

LRESULT VisualizerWindow::MouseLeave(WPARAM wParam, LPARAM lParam)
{
	this->isMouseHovering = false;
	this->isMouseTracking = false;

	static const MARGINS shadow_state{ 0, 0, 0, 0 };
	OK(::DwmExtendFrameIntoClientArea(this->window, &shadow_state));

	return 0;
}

LRESULT VisualizerWindow::RightButtonDown(WPARAM wParam, LPARAM lParam)
{
	HMENU hPopupMenu = ::CreatePopupMenu();
	BET(InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MENU_EXIT, L"Exit"));
	BET(::SetForegroundWindow(this->window));
	POINT point{ LOWORD(lParam), HIWORD(lParam) };
	BET(::TrackPopupMenu(
		hPopupMenu,
		TPM_TOPALIGN | TPM_LEFTALIGN,
		point.x,
		point.y,
		0,
		this->window,
		NULL));
}

LRESULT VisualizerWindow::Command(WPARAM wParam, LPARAM lParam)
{
	// If from menu
	if (HIWORD(wParam) == 0)
	{
		switch (LOWORD(wParam))
		{
		case MENU_EXIT:
			::DestroyWindow(this->window);
			return 0;
		}
	}

	return 0;
}

LRESULT VisualizerWindow::Close()
{
	this->Destroy();
	::PostQuitMessage(0);
	return 0;
}

