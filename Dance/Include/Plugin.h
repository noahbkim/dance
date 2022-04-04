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

    struct Plugin
    {
        size_t Index;
        std::wstring Name;
        Visualizer::Constructor Constructor;
        Visualizer::Destructor Destructor;
    };

    class Plugins
    {
    public:
        static void Load();
        static void Register(const std::wstring& name, const Visualizer::Constructor& constructor, const Visualizer::Destructor& destructor);
        static const std::vector<Plugin>& Get();
        static const Plugin& First();

    private:
        static std::vector<Plugin>& Vector();
    };

    extern "C" __declspec(dllexport) Dance::API::About _Dance();

    extern "C" __declspec(dllexport) void _Register
    (
        const std::wstring& name,
        const Visualizer::Constructor& constructor,
        const Visualizer::Destructor& destructor
    );
}
