#pragma once

#include "Window.h"

class BorderlessWindow : public Window
{
public:
    BorderlessWindow(HINSTANCE instance, std::wstring windowClassName, std::wstring windowTitle);
    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    virtual LRESULT HitTest(LPARAM lParam);
    virtual LRESULT CalculateSize(WPARAM wParam, LPARAM lParam);
};
