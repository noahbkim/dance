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
    INT16 left;
    INT16 right;
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
    float real;
    float imaginary;
};

class FFTWFPlan
{
public:
    FFTWFPlan() : created(false) {}

    FFTWFPlan(fftwf_plan plan) : plan(plan), created(true) {}

    ~FFTWFPlan()
    {
        if (this->created)
        {
            ::fftwf_destroy_plan(this->plan);
        }
    }

    void Overwrite(fftwf_plan plan)
    {
        if (this->created)
        {
            ::fftwf_destroy_plan(this->plan);
        }

        this->plan = plan;
    }

    void Execute() const
    {
        ::fftwf_execute(this->plan);
    }

private:
    fftwf_plan plan;
    bool created;
};

class AudioAnalyzer : public AudioListener
{
public:
    AudioAnalyzer();
    AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    virtual bool Listen();
    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags);
    void Analyze();

    void Sink(fftwf_complex* out);

    size_t Window() const
    {
        return this->window;
    }

protected:
    // Parameters
    size_t window{ 0 };
    size_t rate{ 0 };

    // Buffer
    std::vector<FFTWFComplex> buffer;
    size_t index{ 0 };
    size_t count{ 0 };
    LARGE_INTEGER timestamp{ 0 };

    // FFT
    FFTWFPlan fftwPlan;
    std::vector<FFTWFComplex> fftwBuffer;
};
