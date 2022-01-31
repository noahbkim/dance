// Dance.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "capture.h"
#include "resource.h"
#include "engine.h"

#include <Windows.h>
#include <time.h>
#include <iostream>

using namespace std;

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define MAX_LOADED_STRING_LENGTH 100

struct {
    HINSTANCE Handle = nullptr;
    Engine Engine;
} Window;

INT_PTR CALLBACK ShowAboutDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK HandleWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false;

    Engine* engine = reinterpret_cast<Engine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_ABOUT:
            DialogBox(Window.Handle, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, ShowAboutDialog);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        if (s_in_sizemove) 
        {
            if (engine)
            {
                engine->Tick();
            }
        }
        else 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && engine)
                {
                    engine->OnSuspending();
                }
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && engine)
            {
                engine->OnResuming();
            }
            s_in_suspend = false;
        }
        else if (!s_in_sizemove && engine)
        {
            engine->OnWindowSizeChanged({ 0, 0, LOWORD(lParam), HIWORD(lParam) });
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (engine)
        {
            RECT rectangle;
            GetClientRect(hWnd, &rectangle);
            engine->OnWindowSizeChanged(rectangle);
        }
        break;

    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (engine)
        {
            if (wParam)
            {
                engine->OnActivated();
            }
            else
            {
                engine->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && engine)
            {
                engine->OnSuspending();
            }
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && engine)
                {
                    engine->OnResuming();
                }
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // Implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen)
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                ShowWindow(hWnd, SW_SHOWNORMAL);
                RECT rectangle = engine->GetDefaultSize();

                SetWindowPos(
                    hWnd,
                    HWND_TOP,
                    rectangle.left,
                    rectangle.top,
                    rectangle.right,
                    rectangle.bottom,
                    SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }

            s_fullscreen = !s_fullscreen;
        }
        break;

    case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT RegisterWindowClass(HINSTANCE hInstance, WCHAR* szWindowClass)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = HandleWindowEvent;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DANCE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DANCE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RECT rectangle = Window.Engine.GetDefaultSize();
    AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE);
    return RegisterClassExW(&wcex) ? 0 : 1;
}

HRESULT SetupWindow(HINSTANCE hInstance, int nCmdShow, WCHAR* szWindowClass, WCHAR* szWindowTitle)
{
    HWND hWnd = CreateWindowW(
        szWindowClass,
        szWindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
    {
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&Window.Engine));
    UpdateWindow(hWnd);

    RECT rectangle;
    GetClientRect(hWnd, &rectangle);
    Window.Engine.Setup(hWnd, rectangle);

    return 0;
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    Window.Handle = hInstance;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WCHAR szWindowClass[MAX_LOADED_STRING_LENGTH];
    WCHAR szWindowTitle[MAX_LOADED_STRING_LENGTH];
    LoadStringW(hInstance, IDS_APP_TITLE, szWindowClass, MAX_LOADED_STRING_LENGTH);
    LoadStringW(hInstance, IDS_APP_TITLE, szWindowTitle, MAX_LOADED_STRING_LENGTH);

    GUARD(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED), result) {
        TRACE("failed to initialize threading model: " << result);
        return result;
    }

    GUARD(RegisterWindowClass(hInstance, szWindowClass), result) {
        TRACE("failed to register window class");
        return result;
    }

    GUARD(SetupWindow(hInstance, nCmdShow, szWindowClass, szWindowTitle), result) {
        TRACE("failed to set up window");
        return result;
    }

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
    
    HACCEL hAcceleratorTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DANCE));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAcceleratorTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Window.Engine.Tick();
        }
    }

    Window.Engine.Teardown();
    CoUninitialize();

    return msg.wParam;
}
