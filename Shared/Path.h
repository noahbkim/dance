#pragma once

#include "Options.h"

#include <filesystem>
#include <windows.h>

#define DANCE_MAX_PATH MAX_PATH

/// Get the path of an HMODULE. This function is specifically targeted at determining the source of DLL's loaded via 
/// ::LoadLibrary(), and will return their path regardless of callsite if passed as a pointer. If the pointer is null,
/// the path of the executable is returned.
/// 
/// @param of is the handle of the module or executable to determine that path of.
/// @returns an std::filesystem::path of the module.
static inline std::filesystem::path GetModulePath(HMODULE of = nullptr)
{
    std::wstring moduleFileName;
    moduleFileName.resize(DANCE_MAX_PATH);
    ::GetModuleFileName(of, moduleFileName.data(), DANCE_MAX_PATH);
    moduleFileName.shrink_to_fit();
    return std::filesystem::path(moduleFileName);
}
