#include "VisualizerWindow.h"

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

	TRACE("show shadow");
	static const MARGINS shadow_state{ 1, 1, 1, 1 };
	OK(::DwmExtendFrameIntoClientArea(this->window, &shadow_state));

	return 0;
}

LRESULT VisualizerWindow::MouseLeave(WPARAM wParam, LPARAM lParam)
{
	this->isMouseHovering = false;
	this->isMouseTracking = false;

	TRACE("hide shadow");
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

	return 0;
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
