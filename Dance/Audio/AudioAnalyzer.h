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

/// Whether we create a separate buffer to "un-ring" the contents of the raw audio data buffer before performing the
/// FFT. Unclear whether this is necessary. Maybe if I'd paid attention in complex I'd remember.
// #define ORDER

class AudioAnalyzer : public AudioListener
{
public:
    /// Initialize an empty audio analyzer without performing allocation. Will not work in this state.
    AudioAnalyzer();

    /// Initialize a new audio analyzer for a system audio device and with a buffer duration. Parameters directly
    /// mirror those passed to the AudioListener.
    /// 
    /// @param device expects a ComPtr to a system audio device.
    /// @param duration is a duration in 100 ns intervals corresponding to hnsPeriodicity in IAudioClient::Initialize.
    /// @exception ComError if format determination fails, audio client initialization fails, or capture setup fails.
    AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    /// We override the handle method to write the audio frame to our ring buffer for later analysis. Because this
    /// buffer is written to circularly, it may have to be reordered prior to analysis.
    /// 
    /// @param data the PCM audio frame array recevied from the AudioClient.
    /// @param count the number of frames in the data blob.
    /// @param flags any additional flags yielded by the audio frame.
    virtual void Handle(const PCMAudioFrame* data, UINT32 count, DWORD flags);

    /// Run the fftwf_plan on the data buffer we've been adding to in AudioAnalyzer::Handle.
    void Analyze();

    /// Get a reference to the spectrum data.
    /// 
    /// @returns a const reference to the complex spectrum output.
    const std::vector<FFTWFComplex>& Spectrum() const;

protected:
    // Parameters
    size_t window{ 0 };

    // Buffer
    std::vector<float> buffer;
    size_t index{ 0 };
    size_t count{ 0 };
    LARGE_INTEGER timestamp{ 0 };

    // FFT
    FFTWFPlan fft;
    std::vector<FFTWFComplex> spectrum;

    /// If we wish to order the contents of the ring buffer prior to running the FFT, preallocate that space.
#ifdef ORDER
    std::vector<float> ordered;
#endif
};
