#pragma once

#include "Framework.h"
#include "TransparentWindow.h"
#include "Runtime.h"
#include "Visualizer.h"
#include "Plugin.h"

namespace Dance::Application
{
    /// Main dance visualizer host application. Provides a window and render/update loop for instantiated visualizers
    /// to execute on. Handles user input with regard to window appearance. Invokes plugin logic.
    class VisualizerWindow : public TransparentWindow, public Runtime
    {
    public:
        /// Instantiate a new visualizer window. You must call VisualizerWindow::Create() in order to create the
        /// application window.
        /// 
        /// @param instance should be the programs handle on its own process.
        /// @param windowClassName is a class name to use for the window's style container.
        /// @param windowTitle is the desired title of the wrapped window.
        VisualizerWindow(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

        /// Provide dependencies to visualizer instantion.
        /// 
        /// @returns a POD containing handles to relevant graphical resources.
        virtual const Visualizer::Dependencies& Dependencies() const;

        /// Create the application window, invoking parent creation methods.
        /// 
        /// @returns an HRESULT indicating success.
        virtual HRESULT Create();

        /// Do all necessary event handling for the purposes of our application. In addition to parent class handlers,
        /// bind render, menu command, and mouse event callbacks. 
        /// 
        /// @param windowHandle a handle to our underlying HWND.
        /// @param message the message enum.
        /// @param wParam is the short value parameter.
        /// @param lParam is the long value parameter.
        /// @returns an LRESULT expected by the static callback, sometimes used as a response.
        /// @seealso TransparentWindow::Message.
        virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

        /// Render method invoked on WM_PAINT by VisualizerWindow::Message.
        void Render();

        /// Update method invoked by the main thread via Runtime::Tick.
        /// 
        /// @param delta represents the number of milliseconds that have elapsed since the last call to Update.
        void Update(double delta);

        /// Close and destroy the window.
        /// 
        /// @returns an LRESULT indicating success.
        LRESULT Close();

    protected:
        /// Whether the mouse is currently hovering over any of the hittable area of the window.
        bool isMouseHovering = false;

        /// Whether we're tracking the mouse's location to test hovering.
        bool isMouseTracking = false;

        /// The current visualizer. Managed by originating plugin.
        Visualizer* visualizer;

        /// A managed reference the source plugin of our current visualizer.
        std::reference_wrapper<const Plugin> plugin;

        /// Switch to a different visualizer via its registered plugin.
        /// 
        /// @param plugin should be a constant reference to a plugin from the plugin manager.
        /// @returns an LRESULT to propagate through VisualizerWindow::Command.
        LRESULT Switch(const Plugin& plugin);

        /// Called on resizing the window. Tells the current visualizer to resize as well.
        /// 
        /// @returns an HRESULT indicating success.
        virtual HRESULT Resize();

        /// Handle mouse movement while tracked. Sets hovering state and re-tracks the mouse once the last call has
        /// expired. Invoked by VisualizerWindow::Message.
        /// 
        /// @param wParam is unused in this method. 
        /// @param lParam is unused in this method.
        /// @returns an LRESULT to pass through VisualizerWindow::Message.
        LRESULT MouseMove(WPARAM wParam, LPARAM lParam);

        /// Handle mouse movement while tracked. Sets hovering state. Invoked by VisualizerWindow::Message.
        /// 
        /// @param wParam is unused in this method. 
        /// @param lParam is unused in this method.
        /// @returns an LRESULT to pass through VisualizerWindow::Message.
        LRESULT MouseHover(WPARAM wParam, LPARAM lParam);

        /// Handle mouse movement while tracked. Sets hovering state. Invoked by VisualizerWindow::Message.
        /// 
        /// @param wParam is unused in this method. 
        /// @param lParam is unused in this method.
        /// @returns an LRESULT to pass through VisualizerWindow::Message.
        LRESULT MouseLeave(WPARAM wParam, LPARAM lParam);

        /// Spawn a context menu where the user right-clicked. The selected menu item is handled by 
        /// VisualizerWindow::Command.
        /// 
        /// @param wParam is unused in this method. 
        /// @param lParam contains the packed coordinates of where the user clicked.
        /// @returns an LRESULT to pass through VisualizerWindow::Message.
        LRESULT RightButtonDown(WPARAM wParam, LPARAM lParam);

        /// Handles context menu command selection. Either closes the window and exits cleanly or switches the current
        /// visualizer to whatever's been selected.
        /// 
        /// @param wParam marks whether the command was invoked from the context menu and which item was selected.
        /// @param lParam is unused in this method.
        /// @returns an LRESULT to pass through VisualizerWindow::Message.
        LRESULT Command(WPARAM wParam, LPARAM lParam);
    };
}
