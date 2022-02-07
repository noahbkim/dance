#pragma once

#include "Framework.h"
#include "AudioListener.h"
#include "FFTW3/fftw3.h"

#include <vector>

#pragma comment(lib, "FFTW3/libfftw3f-3.lib")

struct FFTWFComplex
{
    float real;
    float imaginary;

    float magnitude() const
    {
        return std::sqrtf(this->real * this->real + this->imaginary * this->imaginary);
    }

    template<typename T>
    float magnitude(T normalize) const
    {
        float real = this->real / normalize;
        float imaginary = this->imaginary / normalize;
        return std::sqrtf(real * real + imaginary * imaginary);
    }
};

class FFTWFPlan
{
public:
    FFTWFPlan() : plan(), created(false) {}

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

    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags);
    void Analyze();

    void Sink(fftwf_complex* out);

    size_t Window() const
    {
        return this->window;
    }

protected:
    // Parameters
    UINT32 window{ 0 };
    UINT32 rate{ 0 };

    // Buffer
    std::vector<FFTWFComplex> buffer;
    UINT32 index{ 0 };
    UINT32 count{ 0 };
    LARGE_INTEGER timestamp{ 0 };

    // FFT
    FFTWFPlan fftwPlan;
    std::vector<FFTWFComplex> fftwBuffer;
};
