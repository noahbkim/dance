#include "Plugin.h"
#include "Path.h"

namespace Dance::Application
{
    // https://stackoverflow.com/a/42844629
    static bool EndsWith(std::string_view str, std::string_view suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    void Plugins::Load()
    {
        std::filesystem::path visualizersDirectory = GetModulePath().parent_path() / "Visualizers";
        if (!std::filesystem::is_directory(visualizersDirectory))
        {
            return;
        }

        for (const auto& item : std::filesystem::recursive_directory_iterator{ visualizersDirectory })
        {
            if (EndsWith(item.path().string(), ".dll"))
            {
                ::LoadLibrary(item.path().wstring().data());
            }
        }
    }

    void Plugins::Register
    (
        const std::wstring& name,
        const Visualizer::Constructor& constructor,
        const Visualizer::Destructor& destructor
    ) {
        Plugins::Vector().push_back({ Plugins::Vector().size(), name, constructor, destructor });
    }

    const std::vector<Plugin>& Plugins::Get()
    {
        return Plugins::Vector();
    }

    const Plugin& Plugins::First()
    {
        return Plugins::Vector().front();
    }

    std::vector<Plugin>& Plugins::Vector()
    {
        static std::vector<Plugin> plugins;
        return plugins;
    }

    extern "C" __declspec(dllexport) Dance::API::About _Dance()
    {
        return { 0, 0, 1 };
    }

    extern "C" __declspec(dllexport) void _Register(
        const std::wstring& name,
        const Visualizer::Constructor& constructor,
        const Visualizer::Destructor& destructor
    ) {
        Plugins::Register(name, constructor, destructor);
    }
}
