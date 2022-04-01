#pragma once

#include "Framework.h"
#include "Common/Pointer.h"
#include "Audio/Audio.h"
#include "Audio/Device.h"
#include "Audio/AudioAnalyzer.h"
 
#include <functional>
#include <vector>

class Visualizer
{
public:
    struct Dependencies
    {
        // Window
        HINSTANCE Instance;
        HWND Window;

        // TransparentWindow
        ComPtr<ID3D11Device> D3dDevice;
        ComPtr<IDXGIDevice> DxgiDevice;
        ComPtr<IDXGISwapChain1> DxgiSwapChain;
        ComPtr<ID2D1Device1> D2dDevice;
    };

    virtual ~Visualizer();

    // Allocation and deallocation of size-dependent resources
    virtual HRESULT Unsize() = 0;
    virtual HRESULT Resize(const RECT& size) = 0;

    // Runtime hooks
    virtual void Render() = 0;
    virtual void Update(double delta) = 0;
};

class VisualizerRegistry
{
public:
    struct Entry
    {
        using Factory = std::function<std::unique_ptr<Visualizer>(const Visualizer::Dependencies&)>;

        size_t Index;
        std::wstring Name;
        Factory New;

        Entry(size_t index, const std::wstring& name, Factory factory)
            : Index(index)
            , Name(name)
            , New(factory) {}
    };

    template<typename T>
    static size_t Register(const std::wstring& name)
    {
        std::vector<Entry>& entries = VisualizerRegistry::entries();
        size_t index = entries.size();
        entries.emplace_back(
            index, 
            name, 
            [](const Visualizer::Dependencies& dependencies) { return std::make_unique<T>(dependencies); });
        return index;
    }

    static const std::vector<Entry>& Entries()
    {
        return VisualizerRegistry::entries();
    }

private:
    static std::vector<Entry>& entries()
    {
        static std::vector<Entry> entries;
        return entries;
    }
};

#define REGISTER(name, type) static int _##name##_index = VisualizerRegistry::Register<type>(L#name);

class AudioVisualizer : public virtual Visualizer
{
public:
    AudioVisualizer(const Dependencies& dependencies);
    virtual ~AudioVisualizer();

    virtual void Update(double delta);

protected:
    AudioAnalyzer analyzer;
};

class TwoVisualizer : public virtual Visualizer
{
public:
    TwoVisualizer(const Dependencies& dependencies);

    virtual HRESULT Unsize();
    virtual HRESULT Resize(const RECT& size);

protected:
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID2D1Device1> d2dDevice;
    ComPtr<IDXGISurface2> dxgiSurface;
    ComPtr<ID2D1Bitmap1> d2dBitmap;
    ComPtr<ID2D1DeviceContext> d2dDeviceContext;

    HRESULT CreateSurface();
    HRESULT ReleaseSurface();
    HRESULT CreateBitmap();
    HRESULT ReleaseBitmap();
};

class ThreeVisualizer : public virtual Visualizer
{
public:
    ThreeVisualizer(const Dependencies& dependencies);

    virtual HRESULT Unsize();
    virtual HRESULT Resize(const RECT& size);

protected:
    ComPtr<IDXGISwapChain1> dxgiSwapChain;
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;
    ComPtr<ID3D11RenderTargetView> d3dBackBufferView;
    ComPtr<ID3D11DepthStencilView> d3dDepthStencilView;
    ComPtr<ID3D11Texture2D> d3dDepthTexture;
    ComPtr<ID3D11SamplerState> d3dSamplerState;

    HRESULT CreateRenderTarget();
    HRESULT ReleaseRenderTarget();
    HRESULT CreateDepthStencil(const RECT& size);
    HRESULT ReleaseDepthStencil();
};
