#include "Window.h"

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
		WS_EX_APPWINDOW,
		this->windowClassName.c_str(),
		this->windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
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

HRESULT Window::Style(DWORD style)
{
	DWORD current = static_cast<DWORD>(::GetWindowLongPtrW(this->window.get(), GWL_STYLE));
	if (style != current)
	{
		::SetWindowLongPtrW(this->window.get(), GWL_STYLE, static_cast<LONG>(style));

		// Redraw the frame
		BET(::SetWindowPos(this->window.get(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE));
		::ShowWindow(this->window.get(), SW_SHOW);
	}

	return S_OK;
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

LRESULT CALLBACK Window::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		this->Paint();
		return 0;
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

void Window::Paint()
{

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
	, showShadow(true)
{

}

HRESULT BorderlessWindow::Create()
{
	OK(Window::Create());
	this->Style(WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
	return S_OK;
}

LRESULT CALLBACK BorderlessWindow::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCALCSIZE:
		if (wParam == TRUE) 
		{
			auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
			adjustMaximizedClientRectangle(this->window.get(), params.rgrc[0]);
		}
		break;
	case WM_NCHITTEST:
		return this->Hit(lParam);
	default:
		return Window::Message(windowHandle, message, wParam, lParam);
	}

	return 0;
}

void BorderlessWindow::Paint()
{

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

