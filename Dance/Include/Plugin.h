#pragma once

#include <functional>
#include <vector>
#include <filesystem>
#include <utility>
#include <cstdint>

#include "Visualizer.h"
#include "Macro.h"

namespace Dance::Application
{
    using Dance::API::Visualizer;

    /// A plugin is registered by a loaded DLL via the _Register function exposed from the executable runtime. A plugin
    /// must provide its own name, a constructor, and a destructor.
    struct Plugin
    {
        /// The index of the plugin in the plugin vector for convenience.
        size_t Index;

        /// The name of the plugin as displayed to the user.
        std::wstring Name;

        /// A construction method for spawning a new instance of the plugin's visualizer.
        Visualizer::Constructor Constructor;

        /// A matching destructor for the plugin's visualizer.
        Visualizer::Destructor Destructor;
    };

    /// A static singleton for managing plugins loaded at runtime. The private Plugins::Vector method returns a static
    /// vector of all available plugins, which is added to via Plugins::Register. This is in turn called by the extern
    /// _Register functions, which can be found via ::GetProcAddress.
    class Plugins
    {
    public:
        /// Execute ::LoadLibrary on all DLL files in the adjacent Visulizers directory.
        static void Load();

        /// Add a new plugin to the static registrar.
        /// 
        /// @param name is the user-facing name of the provided visualizer.
        /// @param constructor constructs a new visualizer and returns a pointer to it.
        /// @param destructor properly deallocates the pointer returned by the constructor.
        static void Register(const std::wstring& name, const Visualizer::Constructor& constructor, const Visualizer::Destructor& destructor);

        /// Get a constant handle to the static vector of registered plugins.
        /// 
        /// @returns a reference to the plugin vector.
        static const std::vector<Plugin>& Get();

        /// Get the first available plugin, assumes there is at least one in the vector. This is used for convenient
        /// initialization of the initial reference value.
        /// 
        /// @returns the first registered plugin.
        static const Plugin& First();

    private:
        /// Do underlying static trick to get a static vector of plugins.
        ///
        /// @returns a mutable reference to the plugin vector.
        static std::vector<Plugin>& Vector();
    };

    /// Externally visible info method. Used to identify compatibility between plugin and runtime.
    ///
    /// @returns a Dance::API::About, which is a version struct.
    extern "C" __declspec(dllexport) Dance::API::About _Dance();

    /// Externally visible registration method. Shadows Plugins::Register.
    ///
    /// @param name is the user-facing name of the provided visualizer.
    /// @param constructor constructs a new visualizer and returns a pointer to it.
    /// @param destructor properly deallocates the pointer returned by the constructor.
    extern "C" __declspec(dllexport) void _Register
    (
        const std::wstring& name,
        const Visualizer::Constructor& constructor,
        const Visualizer::Destructor& destructor
    );
}
