#pragma once

#include "Framework.h"
#include "BorderlessWindow.h"

namespace Dance::Application
{
    /// A version of the borderless window that renders transparently against the desktop and other windows. Uses the
    /// available DirectX device to create a DXGI interface, swapchain, and composition setup.
    class TransparentWindow : public BorderlessWindow
    {
    public:
        /// Instanatiate a new transparent window using the executable instance, a style class name, and a title.
        /// 
        /// @param instance should be the programs handle on its own process.
        /// @param windowClassName is a class name to use for the window's style container.
        /// @param windowTitle is the desired title of the wrapped window.
        TransparentWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

        /// Modifies the window style to prevent hit redirection from the background, calls BorderlessWindow::Create,
        /// sets up the composition pipeline. This entails creating a DirectX 3D device, creating a DXGI factory and
        /// swapchain for composition, creating a DirectX 2D device, and finalizing the composition components as
        /// handled by TransparentWindow::CreateComposition.
        ///
        /// @returns an HRESULT indicating success, bubbling any error codes from methods invoked inside.
        /// @seealso TransparentWindow::CreateCompmosition.
        virtual HRESULT Create();

        /// Overrides the parent position method to resize the composition and 2D
        ///
        /// @param x is the x coordinate relative to the top left of the main monitor.
        /// @param y is the y coordinate relative to the top right of the main monitor.
        /// @param width is the pixel width of the window.
        /// @param height is the pixel height of the window.
        /// @param flags additional positioning flags.
        /// @returns an HRESULT status indicating success.
        /// @seealso Window::Position
        virtual HRESULT Position(int x, int y, int width, int height, UINT flags);

        /// Respond to WM_ENTERSIZEMOVE and WM_EXITSIZEMOVE so that the composition pipeline is dynamically
        /// with the window.
        ///
        /// @param windowHandle a handle to our underlying HWND.
        /// @param message the message enum.
        /// @param wParam is the short value parameter.
        /// @param lParam is the long value parameter.
        /// @returns an LRESULT expected by the static callback, sometimes used as a response.
        /// @seealso BorderlessWindow::Message.
        virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    protected:
        /// Underlying DirectX device we use for graphical invocations.
        ComPtr<ID3D11Device> d3dDevice;

        /// Cast of this->d3dDevice as a DXGI device for integrating 2D graphics.
        ComPtr<IDXGIDevice> dxgiDevice;

        /// Factory for the DXGI components we will use in our composition pipeline.
        ComPtr<IDXGIFactory2> dxgiFactory;

        /// A swapchain we'll use to present the everything we render to our window surface, including the
        /// data generated by the composition pipeline.
        ComPtr<IDXGISwapChain1> dxgiSwapChain;

        /// A DirectX 2D component factory we'll use to set up 2D rendering on our window.
        ComPtr<ID2D1Factory2> d2dFactory;

        /// The composition device that handles rendering the background of our surface into our swapchain.
        ComPtr<IDCompositionDevice> dCompositionDevice;

        /// The container for our window handle as a target for composition data.
        ComPtr<IDCompositionTarget> dCompositionTarget;

        /// Indicates that we're rendering the composition to our swapchain.
        ComPtr<IDCompositionVisual> dCompositionVisual;

        /// Set up the composition pipeline per the example in the linked article.
        ///
        /// @returns an HRESULT status indicating success.
        /// @seealso https://docs.microsoft.com/en-us/archive/msdn-magazine/2014/june/windows-with-c-high-performance-window-layering-using-the-windows-composition-engine
        HRESULT CreateComposition();

        /// A persistent container for our window size as reported by ::GetClientRect. Updated when
        /// TransparentWindow::Resize is invoked.
        RECT size{};

        /// Override to resize our DXGI buffers after adjusting the window geometry.
        ///
        /// @returns an HRESULT status indicating success.
        virtual HRESULT Resize();

        /// Indicates whether the user is actively resizing or moving the window.
        bool isResizingOrMoving = false;

        /// Invoked by the TransparentWindow::Message handler, manages the isResizingOrMoving flag.
        ///
        /// @returns an HRESULT status indicating success.
        virtual LRESULT StartResizeMove();

        /// Invoked by the TransparentWindow::Message handler, manages the isResizingOrMoving flag.
        ///
        /// @returns an HRESULT status indicating success.
        virtual LRESULT FinishResizeMove();
    };
}
