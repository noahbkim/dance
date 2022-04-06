#include "VisualizerWindow.h"

namespace Dance::Application
{
	static const WORD MENU_EXIT = -1;
	static const MARGINS SHADOW_VISIBLE{ 1, 1, 1, 1 };
	static const MARGINS SHADOW_INVISIBLE{ 0, 0, 0, 0 };

	VisualizerWindow::VisualizerWindow
	(
		InstanceHandle instance,
		std::wstring windowClassName,
		std::wstring windowTitle
	)
		: TransparentWindow(instance, windowClassName, windowTitle)
		, Runtime()
		, visualizer(nullptr)
		, plugin(Plugins::First())
	{

	}

	VisualizerWindow::~VisualizerWindow()
	{
		// Destroy the visualizer
		this->plugin.get().Destructor(this->visualizer);
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
		this->Switch(Plugins::First());
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

	LRESULT VisualizerWindow::Switch(const Plugin& plugin)
	{
		if (this->visualizer != nullptr)
		{
			this->plugin.get().Destructor(this->visualizer);
		}

		this->plugin = plugin;
		this->visualizer = plugin.Constructor(this->Dependencies());
		::SetWindowText(this->window, plugin.Name.data());
		return 0;
	}

	LRESULT VisualizerWindow::MouseMove(WPARAM wParam, LPARAM lParam)
	{
		this->isMouseHovering = true;
		OK(::DwmExtendFrameIntoClientArea(this->window, &SHADOW_VISIBLE));

		// Track again
		if (!this->isMouseTracking)
		{
			TRACKMOUSEEVENT tracking{};
			tracking.cbSize = sizeof(tracking);
			tracking.dwFlags = TME_NONCLIENT | TME_LEAVE;
			tracking.hwndTrack = this->window;
			tracking.dwHoverTime = HOVER_DEFAULT;
			::TrackMouseEvent(&tracking);
			this->isMouseTracking = true;
		}
		return 0;
	}

	LRESULT VisualizerWindow::MouseHover(WPARAM wParam, LPARAM lParam)
	{
		this->isMouseHovering = true;
		OK(::DwmExtendFrameIntoClientArea(this->window, &SHADOW_VISIBLE));
		return 0;
	}

	LRESULT VisualizerWindow::MouseLeave(WPARAM wParam, LPARAM lParam)
	{
		this->isMouseHovering = false;
		this->isMouseTracking = false;

		// The mouse technically leaves if we move or resize the window but we still want to see borders.
		if (!this->isResizingOrMoving)
		{
			OK(::DwmExtendFrameIntoClientArea(this->window, &SHADOW_INVISIBLE));
		}

		return 0;
	}

	LRESULT VisualizerWindow::RightButtonDown(WPARAM wParam, LPARAM lParam)
	{
		HMENU menu = ::CreatePopupMenu();
		for (const Plugin& plugin : Plugins::Get())
		{
			BET(::AppendMenu(
				menu,
				MF_BYPOSITION | MF_STRING | (this->plugin.get().Index == plugin.Index ? MF_CHECKED | MF_DISABLED : 0),
				plugin.Index,
				plugin.Name.data()));
		}

		// Exit
		BET(::AppendMenu(
			menu,
			MF_BYPOSITION | MF_STRING,
			MENU_EXIT,
			L"Exit"));

		BET(::SetForegroundWindow(this->window));
		BET(::TrackPopupMenu(
			menu,
			TPM_TOPALIGN | TPM_LEFTALIGN,
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam),
			0,
			this->window,
			nullptr));

		return 0;
	}

	LRESULT VisualizerWindow::Command(WPARAM wParam, LPARAM lParam)
	{
		// If from menu
		if (HIWORD(wParam) == 0)
		{
			WORD index = LOWORD(wParam);
			if (index == MENU_EXIT)
			{
				::DestroyWindow(this->window);
				return 0;
			}
			else
			{
				return this->Switch(Plugins::Get().at(index));
			}
		}

		return 0;
	}

	LRESULT VisualizerWindow::Close()
	{
		// Close the window
		this->Destroy();

		// We quit when this window closes
		::PostQuitMessage(0);

		return 0;
	}
}
