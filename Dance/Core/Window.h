#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

class Window
{
public:
    Window(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual ~Window();
    virtual HRESULT Create();
    virtual HRESULT Prepare(int showCommand);
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void Paint();

    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);
    static LRESULT CALLBACK Global(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    static void Main(InstanceHandle instance);

protected:
    InstanceHandle instance;
    WindowHandle window;
    std::wstring windowClassName;
    std::wstring windowTitle;

    virtual WNDCLASSEXW Class();
    virtual HRESULT Register();
    virtual HRESULT Style(DWORD style);
};

class BorderlessWindow : public Window
{
public:
    BorderlessWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void Paint();

protected:
    virtual LRESULT Hit(LPARAM lParam);

private:
    bool showShadow;
};
