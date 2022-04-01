#pragma once

#include <functional>
#include <vector>
#include <filesystem>

#include "Visualizer.h"

class VisualizerRegistry
{
public:
    struct Entry
    {
        size_t Index;
        std::wstring Name;
        std::function<Visualizer*(const Visualizer::Dependencies&, const std::filesystem::path&)> Factory;
        std::filesystem::path Path;

        Entry(size_t index, const std::wstring& name, std::function<Visualizer*(const Visualizer::Dependencies&, const std::filesystem::path& path)> factory, const std::filesystem::path& path)
            : Index(index)
            , Name(name)
            , Factory(factory) 
            , Path(path) {}

        std::unique_ptr<Visualizer> Create(const Visualizer::Dependencies& dependencies) const
        {
            return std::unique_ptr<Visualizer>(this->Factory(dependencies, this->Path));
        }
    };

    static HRESULT Load(const std::wstring& path)
    {
        HINSTANCE library = LoadLibrary(path.data());
        if (library == nullptr)
        {
            TRACE("couldn't load plugin from " << path);
            return E_FAIL;
        }

        Visualizer::New factory = reinterpret_cast<Visualizer::New>(GetProcAddress(library, "Factory"));
        if (factory == nullptr)
        {
            TRACE("couldn't load factory from plugin " << path);
            return E_FAIL;
        }

        Visualizer::Name name = reinterpret_cast<Visualizer::Name>(GetProcAddress(library, "Name"));
        if (name == nullptr)
        {
            TRACE("couldn't load name from plugin " << path);
            return E_FAIL;
        }

        std::vector<Entry>& entries = VisualizerRegistry::entries();
        size_t index = entries.size();
        entries.emplace_back(index, name(), *factory, path);

        return S_OK;
    }

    static HRESULT Load()
    {

        std::wstring moduleFileName;
        moduleFileName.resize(MAX_PATH);
        ::GetModuleFileName(nullptr, moduleFileName.data(), moduleFileName.size());
        moduleFileName.shrink_to_fit();

        std::filesystem::path executableDirectory = std::filesystem::path(moduleFileName).parent_path();
        for (const auto& item : std::filesystem::recursive_directory_iterator{ executableDirectory / "Visualizers"})
        {
            std::wstring itemPath = item.path().wstring();
            if (itemPath.rfind(L".dll") != std::string::npos)
            {
                TRACE(itemPath);
                VisualizerRegistry::Load(std::wstring(itemPath));
            }
        }

        return S_OK;
    }

    static const std::vector<Entry>& Entries()
    {
        return VisualizerRegistry::entries();
    }

private:
    static std::vector<Entry>& entries()
    {
        static std::vector<Entry> entries;
        return entries;
    }
};
