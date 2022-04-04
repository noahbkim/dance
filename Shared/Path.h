#pragma once

#include "Options.h"

#include <filesystem>
#include <windows.h>

static inline std::filesystem::path GetModulePath(HMODULE of = nullptr)
{
    std::wstring moduleFileName;
    moduleFileName.resize(MAX_PATH);
    ::GetModuleFileName(of, moduleFileName.data(), moduleFileName.size());
    moduleFileName.shrink_to_fit();
    return std::filesystem::path(moduleFileName);
}
