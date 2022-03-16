#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

/// Base window class that wraps HWND and relate functionality. Methods that represent the window's lifecycle are 
/// declared in the order they're intended to be called.
class Window
{
public:
    /// Instanatiate a new window using the executable instance, a style class name, and a title.
    /// 
    /// @param instance should be the programs handle on its own process.
    /// @param windowClassName is a class name to use for the window's style container.
    /// @param windowTitle is the desired title of the wrapped window.
    Window(HINSTANCE instance, std::wstring windowClassName, std::wstring windowTitle);

    /// Virtual destructor for inheritors.
    virtual ~Window();

    /// Register a style class and create the underlying window. Finally, register Window::Dispatch as the window
    /// subclass event loop callback so that we can handle our own events.
    /// 
    /// @returns an HRESULT indicating success, bubbling any error codes from methods invoked inside.
    virtual HRESULT Create();

    /// Prepare the window for display. Shows the window and update it to redraw if necessary.
    /// 
    /// @param showCommand indicates how the window should be displayed.
    /// @returns an HRESULT indicating success.
    /// @seealso https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
    virtual HRESULT Prepare(int showCommand);

    /// Set the position and size of the window. Passes additional positioning flags.
    /// 
    /// @param x is the x coordinate relative to the top left of the main monitor.
    /// @param y is the y coordinate relative to the top right of the main monitor.
    /// @param width is the pixel width of the window.
    /// @param height is the pixel height of the window.
    /// @param flags additional positioning flags.
    /// @returns an HRESULT status indicating success.
    /// @seealso https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
    virtual HRESULT Position(int x, int y, int width, int height, UINT flags);

    /// Handle a window event specific to the window we wrap.
    /// 
    /// @param windowHandle a handle to our underlying HWND.
    /// @param message the message enum.
    /// @param wParam is the short value parameter.
    /// @param lParam is the long value parameter.
    /// @returns an LRESULT expected by the static callback, sometimes used as a response.
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    /// Destroy the window via its handle.
    /// 
    /// @returns a status indicating success of window destruction.
    virtual HRESULT Destroy();

    /// Static dispatch for all subclassed instances of this window. We use the owner pointer to invoke Window::Message
    /// on the corresponding Window object. The traditional method for attaching a callback, i.e. providing the 
    /// WNDLCASSEXW initializer with an lpfnWndProc, does not provide a guaranteed way to attach a reference to our
    /// wrapping Window object. On the other hand, ::SetWindowSubclass does via dwRefData.
    /// 
    /// @param windowHandle a handle to our underlying HWND.
    /// @param message the message enum.
    /// @param wParam is the short value parameter.
    /// @param lParam is the long value parameter.
    /// @param subclass is the identifier of the subclass we registered this window handle as.
    /// @param owner is the Window pointer we specified when registering this callback in Window::Create.
    /// @returns the LRESULT returned by the delegated Message call.
    /// @seealso https://stackoverflow.com/questions/117792/best-method-for-storing-this-pointer-for-use-in-wndproc
    /// @seealso https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-procedures
    static LRESULT CALLBACK Dispatch(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclass, DWORD_PTR owner);

protected:
    /// Reference to the executable instance.
    HINSTANCE instance;
    
    /// Handle of our actual window.
    HWND window;

    /// String registered with our custom window style.
    std::wstring windowClassName;

    /// String passed as the title of the instantiated window.
    std::wstring windowTitle;

    /// Window style passed on instantiation for extension options.
    DWORD windowExtensionStyle = WS_EX_OVERLAPPEDWINDOW;

    /// Window style passed on instantiation for standard options.
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;

    /// Overridable shorthand for creating the window class. Allows inheritors to customize window creation.
    /// 
    /// @return a window extension clas
    virtual WNDCLASSEXW Class();

    /// Register the window class. Calls Window::Class.
    /// 
    /// @returns an HRESULT status indicating success.
    virtual HRESULT Register();
};
