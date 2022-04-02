#pragma once

#include <string>
#include <stringapiset.h>

// https://stackoverflow.com/a/6691829
// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
inline std::wstring Widen(const std::string& s)
{
    if (s.empty())
    {
        return std::wstring();
    }

    size_t size = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), 0, 0);

    std::wstring wide(size, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), wide.data(), wide.length());
    return wide;
}
