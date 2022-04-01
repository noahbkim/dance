#include "BorderlessWindow.h"

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
	HINSTANCE instance,
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
		return Window::Message(windowHandle, message, wParam, lParam);
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
