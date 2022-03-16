#pragma once

#include "Window.h"

/// A window that doesn't display its border or menu bar. The main differences from the standard window are in the
/// Window::windowStyle, Window::windowExtensionStyle, and the handlers for the hit detection and size calculation
/// window events.
/// 
/// @seealso https://github.com/melak47/BorderlessWindow/blob/master/BorderlessWindow/src/BorderlessWindow.cpp
class BorderlessWindow : public Window
{
public:
    /// Instanatiate a new borderless window using the executable instance, a style class name, and a title.
    /// 
    /// @param instance should be the programs handle on its own process.
    /// @param windowClassName is a class name to use for the window's style container.
    /// @param windowTitle is the desired title of the wrapped window.
    BorderlessWindow(HINSTANCE instance, std::wstring windowClassName, std::wstring windowTitle);

    /// Modifies the window style, calls Window::Create, and redraws the window so that the changes to the border take
    /// effect prior to display.
    /// 
    /// @returns an HRESULT indicating success, bubbling any error codes from methods invoked inside.
    virtual HRESULT Create();

    /// Respond to WM_NCCALCSIZE and WM_NCHITTEST taking into account the new window geometry.
    /// 
    /// @param windowHandle a handle to our underlying HWND.
    /// @param message the message enum.
    /// @param wParam is the short value parameter.
    /// @param lParam is the long value parameter.
    /// @returns an LRESULT expected by the static callback, sometimes used as a response.
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    /// Determine whether a coordinate lands on the window, and if so, where.
    /// 
    /// @param lParam is the packed desktop coordinates of the point we're testing.
    /// @returns the HT_ enumeration indicating where the coordinates hit.
    virtual LRESULT HitTest(LPARAM lParam);

    /// Calculate the size of the fullscreen window without the menu bar or borders.
    /// 
    /// @param wParam flags whether we're fullscreen.
    /// @param lParam is the packed CALCSIZE parameter set.
    /// @returns zero.
    virtual LRESULT CalculateSize(WPARAM wParam, LPARAM lParam);
};
