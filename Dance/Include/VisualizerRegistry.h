#pragma once

#include <functional>
#include <vector>
#include <filesystem>

#include "Visualizer.h"
#include "Macro.h"
#include "Pointer.h"

namespace Dance::Application
{
    class VisualizerRegistry
    {
    public:
        class Entry
        {
        public:
            size_t Index;
            std::filesystem::path Path;
            std::wstring Name;

            Entry(size_t index, const std::filesystem::path& path, const std::wstring& name, HMODULE library, Visualizer::Factory Factory);

            std::unique_ptr<Visualizer> New(const Visualizer::Dependencies& dependencies) const;

        private:
            LibraryHandle library;
            std::function<Visualizer* (const Visualizer::Dependencies&, const std::filesystem::path&)> factory;
        };

        std::vector<Entry> Entries;

        HRESULT Load(const std::filesystem::path& path);
        HRESULT Load();
    };
}
