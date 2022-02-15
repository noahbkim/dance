#pragma once

#include "Framework.h"
#include "AudioListener.h"

#include "FFTW3/fftw3.h"
#pragma comment(lib, "FFTW3/libfftw3f-3.lib")

/// A complex audio frame consists of a real part and imaginary part. These do not correspond to left and right; 
/// instead, the real part is used for audio magnitude and the imaginary part should be zero.
struct FFTWFComplex
{
    float real;
    float imaginary;

    /// Compute the magnitude of the represented complex number.
    /// 
    /// @returns a real magnitude.
    float magnitude() const
    {
        return std::sqrtf(this->real * this->real + this->imaginary * this->imaginary);
    }

    /// Compute the magnitude of the represented complex number after dividing the components by the provided value.
    /// This is handy because the FFTWF returns an array scaled by N.
    /// 
    /// @typeparam T provides polymorphism on the normalization value.
    /// @param normalize is the value to divide each component by prior to computing the magnitude.
    /// @returns a real magnitude.
    /// @seealso https://www.fftw.org/fftw3_doc/Complex-One_002dDimensional-DFTs.html
    template<typename T>
    float magnitude(T normalize) const
    {
        float real = this->real / normalize;
        float imaginary = this->imaginary / normalize;
        return std::sqrtf(real * real + imaginary * imaginary);
    }
};

/// Managed wrapper for an FFTWFPlan.
class FFTWFPlan
{
public:
    /// Empty initialize an FFTWFPlan without allocating anything. Will throw a nullptr exception if used in this state
    /// as there is no underlying value.
    FFTWFPlan() : plan(), created(false) {}

    /// Instantiate a new FFTWFPlan from the underlying value.
    /// 
    /// @param plan the fftwf_plan to manage.
    FFTWFPlan(fftwf_plan plan) : plan(plan), created(true) {}

    /// Destroy the underlying plan if it was ever passed in.
    ~FFTWFPlan()
    {
        if (this->created)
        {
            ::fftwf_destroy_plan(this->plan);
        }
    }

    /// Overwrite the currently managed plan with a new one. Releases the previous one.
    /// 
    /// @param plan the new fftwf_plan to manage.
    void Overwrite(fftwf_plan plan)
    {
        if (this->created)
        {
            ::fftwf_destroy_plan(this->plan);
        }

        this->plan = plan;
    }

    /// Invoke fftwf_execute on the managed plan. Does not check if the managed plan has been instantiated.
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

    virtual void Handle(const PCMAudioFrame* data, UINT32 count, DWORD flags);
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
