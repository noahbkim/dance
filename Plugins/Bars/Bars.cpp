#pragma once

#include "Bars.h"
#include "Path.h"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        Dance::API::Register(L"Bars", Visualizer::New<BarsVisualizer>, Visualizer::Delete);
    }
    return TRUE;
}
