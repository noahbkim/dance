#include "Window.h"

namespace Dance::Application
{
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

	LRESULT CALLBACK Window::Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR dwRefData)
	{
		// https://stackoverflow.com/questions/35178779/wndproc-as-class-method
		Window* self = reinterpret_cast<Window*>(dwRefData);
		return self->Message(windowHandle, message, wParam, lParam);
	}

	HRESULT Window::Destroy()
	{
		::DestroyWindow(this->window);
		return S_OK;
	}
}
