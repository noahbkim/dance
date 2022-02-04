#pragma once

#include "Framework.h"
#include "Common/Pointer.h"
#include "FFTW3/fftw3.h"

#include <exception>
#include <algorithm>
#include <vector>

#pragma comment(lib, "Winmm.lib")

// A REFERENCE_TIME increment is 100 nanoseconds
#define ONE_SECOND (1e9 / 1e2)
#define ONE_MILLISECOND (1e9 / 1e3 / 1e2)

ComPtr<IMMDevice> getDefaultDevice();
std::wstring getDeviceId(ComPtr<IMMDevice> device);
std::wstring getDeviceFriendlyName(ComPtr<IMMDevice> device);

using PCMAudioFrame = INT16[2];

class AudioListener
{
public:
    AudioListener();
    AudioListener(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    void Start();
    void Stop();

protected:
    ComPtr<IMMDevice> mmDevice;
    ComPtr<IAudioClient> audioClient = nullptr;
    ComPtr<IAudioCaptureClient> audioCaptureClient = nullptr;
    std::unique_ptr<WAVEFORMATEX, CoTaskDeleter<WAVEFORMATEX>> waveFormat = nullptr;

    DWORD interval{ 0 };

    UINT32 bufferSize{ 0 };
    REFERENCE_TIME bufferDuration{ 0 };

    virtual HRESULT Listen();
    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags) = 0;
    
private:
    HRESULT DetermineWaveFormat();
    bool listening = false;
};

class AudioAnalyzer : public AudioListener
{
public:
    AudioAnalyzer();
    AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration, size_t window);

    virtual HRESULT Listen();
    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags);

    static DWORD WINAPI Thread(LPVOID lpParam);

protected:
    // Parameters
    size_t window{ 0 };
    size_t rate{ 0 };

    // Buffer
    std::vector<fftwf_complex> data;
    size_t index{ 0 };
    size_t count{ 0 };
    LARGE_INTEGER timestamp{ 0 };

    // State
    size_t bookmarkIndex{ 0 };
    LARGE_INTEGER bookmarkTimestamp{ 0 };
};
