#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMCX
#define NOSERVICE
#define NOHELP

#include <string>
#include <windows.h>

namespace Dance::Library
{
    template<typename T>
    static inline T* Find(HMODULE library, const std::string& symbol)
    {
        return reinterpret_cast<T*>(::GetProcAddress(library, symbol.c_str()));
    }
}
