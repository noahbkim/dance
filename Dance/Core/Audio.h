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

struct PCMAudioFrame
{
    INT16 l;
    INT16 r;
};

class AudioListener
{
public:
    AudioListener();
    AudioListener(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    virtual HRESULT Enable();
    virtual bool Listen();
    virtual HRESULT Disable();

protected:
    ComPtr<IMMDevice> mmDevice;
    ComPtr<IAudioClient> audioClient = nullptr;
    ComPtr<IAudioCaptureClient> audioCaptureClient = nullptr;
    std::unique_ptr<WAVEFORMATEX, CoTaskDeleter<WAVEFORMATEX>> waveFormat = nullptr;

    UINT32 bufferSize{ 0 };
    REFERENCE_TIME bufferDuration{ 0 };

    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags) = 0;
    
private:
    HRESULT DetermineWaveFormat();
    bool listening = false;
};

struct FFTWFComplex
{
    float l;
    float r;
};

class AudioAnalyzer : public AudioListener
{
public:
    AudioAnalyzer();
    AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    virtual bool Listen();
    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags);
    void Analyze(fftwf_complex* out);

    size_t Window() const
    {
        return this->window;
    }

protected:
    // Parameters
    size_t window{ 0 };
    size_t rate{ 0 };

    // Buffer
    std::vector<FFTWFComplex> data;
    size_t index{ 0 };
    size_t count{ 0 };
    LARGE_INTEGER timestamp{ 0 };

    // FFT
    fftwf_plan plan;
    std::vector<FFTWFComplex> in;
};
