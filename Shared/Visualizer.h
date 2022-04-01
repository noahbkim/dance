#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMCX
#define NOSERVICE
#define NOHELP

#include <windows.h>
#include <wrl.h>
#include <wingdi.h>
#include <d3d11_4.h>
#include <d2d1_2.h>

#include <string>
#include <filesystem>

class Visualizer
{
public:
    // What we expect to be passed to the creation function
    struct Dependencies
    {
        // Window
        HINSTANCE Instance;
        HWND Window;

        // TransparentWindow
        Microsoft::WRL::ComPtr<ID3D11Device> D3dDevice;
        Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> DxgiSwapChain;
        Microsoft::WRL::ComPtr<ID2D1Device1> D2dDevice;
    };

    // Make destructor virtual for children classes.
    virtual ~Visualizer() {}

    // Allocation and deallocation of size-dependent resources
    virtual HRESULT Unsize() = 0;
    virtual HRESULT Resize(const RECT& size) = 0;

    // Runtime hooks
    virtual void Render() = 0;
    virtual void Update(double delta) = 0;

    // Import types
    typedef Visualizer* (__cdecl* New)(const Visualizer::Dependencies&, const std::filesystem::path&);
    typedef std::wstring(__cdecl* Name)();
};

#define VISUALIZER(name, type) \
extern "C" \
{ \
    __declspec(dllexport) Visualizer* New(const Visualizer::Dependencies& dependencies, const std::filesystem::path& path) \
    { \
        return new type(dependencies, path); \
    } \
    __declspec(dllexport) std::wstring Name() \
    { \
        return L#name; \
    } \
}
