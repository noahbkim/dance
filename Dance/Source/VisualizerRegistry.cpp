#include "VisualizerRegistry.h"

namespace Dance::Application
{
    static std::filesystem::path GetModulePath(HMODULE of = nullptr)
    {
        std::wstring moduleFileName;
        moduleFileName.resize(MAX_PATH);
        ::GetModuleFileName(of, moduleFileName.data(), moduleFileName.size());
        moduleFileName.shrink_to_fit();
        return std::filesystem::path(moduleFileName);
    }

    template<typename T>
    static T* Find(HMODULE library, const std::string& symbol)
    {
        return reinterpret_cast<T*>(::GetProcAddress(library, symbol.c_str()));
    }

    // https://stackoverflow.com/a/42844629
    static bool EndsWith(std::string_view str, std::string_view suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    VisualizerRegistry::Entry::Entry
    (
        size_t index,
        const std::filesystem::path& path,
        const std::wstring& name,
        HMODULE library,
        Visualizer::Factory factory
    )
        : Index(index)
        , Path(path)
        , Name(name)
        , library(library)
        , factory(factory)
    {

    }

    std::unique_ptr<Visualizer> VisualizerRegistry::Entry::New(const Visualizer::Dependencies& dependencies) const
    {
        return std::unique_ptr<Visualizer>(this->factory(dependencies, this->Path));
    }

    HRESULT VisualizerRegistry::Load(const std::filesystem::path& path)
    {
        TRACE("loading visualizer from " << path);

        HMODULE library = ::LoadLibrary(path.wstring().data());
        if (library == nullptr)
        {
            TRACE("failed to load visualizer from " << path);
            ::FreeLibrary(library);
            return E_FAIL;
        }

        Visualizer::Factory* factory = Find<Visualizer::Factory>(library, "Factory");
        Visualizer::Name* name = Find<Visualizer::Name>(library, "Name");
        if (factory == nullptr || name == nullptr)
        {
            TRACE("library does not conform to visualizer API " << path);
            ::FreeLibrary(library);
            return E_FAIL;
        }

        size_t index = this->Entries.size();
        this->Entries.emplace_back(index, path, name(), library, *factory);
        return S_OK;
    }

    HRESULT VisualizerRegistry::Load()
    {
        std::filesystem::path executableDirectory = GetModulePath().parent_path();
        for (const auto& item : std::filesystem::recursive_directory_iterator{ executableDirectory / "Visualizers" })
        {
            if (EndsWith(item.path().string(), ".dll"))
            {
                TRACE(item.path());
                this->Load(item.path());
            }
        }
        return S_OK;
    }
}
