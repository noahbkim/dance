#include "Framework.h"
#include "Visualizer.h"

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

    VisualizerWindow visualizer(instance, L"VisualizerWindow", L"Dance");
    OK(visualizer.Create());
    OK(visualizer.Position(100, 100, 480, 480, SWP_FRAMECHANGED));
    OK(visualizer.Prepare(showCommand));

    HACCEL acceleratorTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDC_DANCE));
    MSG message;
    while (GetMessage(&message, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(message.hwnd, acceleratorTable, &message))
        {
            ::TranslateMessage(&message);
            DispatchMessage(&message);
        }

        visualizer.Tick();
    }

    ::CoUninitialize();

    return 0;
}
