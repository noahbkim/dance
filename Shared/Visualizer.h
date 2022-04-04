#pragma once

#include "Options.h"

#include <windows.h>
#include <wrl.h>
#include <wingdi.h>
#include <d3d11_4.h>
#include <d2d1_2.h>

#include <string>
#include <filesystem>
#include <functional>
#include <tuple>

namespace Dance::API
{
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

        // Signatures
        using Constructor = std::function<Visualizer* (const Visualizer::Dependencies&)>;
        using Destructor = std::function<void(Visualizer*)>;

        // Static convenience methods for registration
        template<typename T>
        static inline T* New(const Dependencies& dependencies)
        {
            return new T(dependencies);
        }

        static inline void Delete(Visualizer* t)
        {
            return delete t;
        }
    };

    struct About
    {
        struct {
            uint16_t Major;
            uint16_t Minor;
            uint32_t Patch;
        } Version;
    };

    template<typename Return, typename ...Args>
    static inline Return Defer(LPCSTR to, Args ...args)
    {
        using Signature = Return __cdecl(Args...);
        Signature* actual = reinterpret_cast<Signature*>(::GetProcAddress(::GetModuleHandle(nullptr), to));
        return actual(args...);
    }

    static inline About Dance()
    {
        return Defer<About>("_Dance");
    }

    static inline void Register
    (
        const std::wstring& name,
        const Visualizer::Constructor& constructor,
        const Visualizer::Destructor& destructor
    ) {
        Defer<void, const std::wstring&, const Visualizer::Constructor&, const Visualizer::Destructor&>("_Register", name, constructor, destructor);
    }
}
