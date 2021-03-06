#include "Framework.h"
#include "VisualizerWindow.h"

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define MAX_LOADED_STRING_LENGTH 100

int APIENTRY wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE previousInstance,
    _In_ LPWSTR commandLine,
    _In_ int showCommand)
{
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);

    Dance::Application::Plugins::Load();
    if (Dance::Application::Plugins::Get().size() == 0)
    {
        ::MessageBox
        (
            nullptr,
            L"Make sure your visualizer plugins are available in the Visualizers folder next to the Dance executable.",
            L"No visualizer plugins available!",
            MB_ICONEXCLAMATION| MB_OK
        );

        return 0;
    }

    WCHAR szWindowTitle[MAX_LOADED_STRING_LENGTH];
    ::LoadStringW(instance, IDS_APP_TITLE, szWindowTitle, MAX_LOADED_STRING_LENGTH);

    GUARD(::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED), result) {
        TRACE("failed to initialize threading model: " << result);
        return result;
    }

    Dance::Application::VisualizerWindow window(instance, L"VisualizerWindow", L"Dance");
    OK(window.Create());
    OK(window.Position(100, 100, 480, 480, SWP_FRAMECHANGED));
    OK(window.Prepare(showCommand));

    HACCEL acceleratorTable = ::LoadAccelerators(instance, MAKEINTRESOURCE(IDC_DANCE));
    MSG message;
    while (::GetMessage(&message, nullptr, 0, 0))
    {
        if (!::TranslateAccelerator(message.hwnd, acceleratorTable, &message))
        {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
        }

        window.Tick();
    }

    ::CoUninitialize();

    return 0;
}
