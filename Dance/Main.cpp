// Dance.cpp : Defines the entry point for the application.
//

#include <time.h>

#include "Framework.h"
#include "Core/Window.h"
#include "Engine/Engine.h"

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define MAX_LOADED_STRING_LENGTH 100

int APIENTRY wWinMain(
    _In_ InstanceHandle instance,
    _In_opt_ InstanceHandle previousInstance,
    _In_ LPWSTR commandLine,
    _In_ int showCommand)
{
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);

    WCHAR szWindowTitle[MAX_LOADED_STRING_LENGTH];
    LoadStringW(instance, IDS_APP_TITLE, szWindowTitle, MAX_LOADED_STRING_LENGTH);

    GUARD(::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED), result) {
        TRACE("failed to initialize threading model: " << result);
        return result;
    }

    /*
    try 
    {
        Capture capture(getDefaultDevice());
        capture.Start();
        capture.Debug();
        capture.Stop();
    }
    catch (CaptureException& exception) 
    {
        TRACE("error setting up audio capture: " << exception.hresult);
        return exception.hresult;
    }
    */

    VisualizerWindow window(instance, L"VisualizerWindow", L"Dance");
    OK(window.Create());
    OK(window.Prepare(showCommand));

    Window::Main(instance);
    
    ::CoUninitialize();

    return 0;
}
