#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

class Window
{
public:
    Window(HINSTANCE instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual ~Window();
    virtual HRESULT Create();
    virtual HRESULT Prepare(int showCommand);
    virtual HRESULT Position(int x, int y, int width, int height, UINT flags);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT Destroy();

    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);

protected:
    HINSTANCE instance;
    HWND window;
    std::wstring windowClassName;
    std::wstring windowTitle;
    DWORD windowExtensionStyle = WS_EX_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;

    virtual WNDCLASSEXW Class();
    virtual HRESULT Register();
};
