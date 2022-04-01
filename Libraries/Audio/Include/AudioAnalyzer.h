#pragma once

#include "Common.h"
#include "Ring.h"
#include "Pointer.h"
#include "AudioListener.h"

#include <cmath>

#include "fftw3.h"

namespace Dance::Audio
{
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

    /// Superclass for a converter from an audio frame payload to a flat array of floats. The current visualizer 
    /// implementations expect values to range from 0.0 to 1.0.
    class AudioAdapter
    {
    public:
        /// Write a payload of count frames into the destination ring.
        /// 
        /// @param destination is the buffer of floats we're writing the audio frames to.
        /// @param source is an array of data subclasses should specialize to, determined by the audio analyzer.
        /// @param count indicates how many frames are present in the source array.
        virtual void Write(Ring<float>& destination, const void* source, size_t count) = 0;
    };

    /// Statically casts between primitive value types. Provides normalization factor to divide integer audio
    /// frames into [0.0, 1.0] floating point values.
    /// 
    /// @typeparam T is the parameter we're casting our const void* source to.
    template<typename T>
    class StaticAudioAdapter : public AudioAdapter
    {
    public:
        /// A static audio adapter needs to know how many channels there are in each frame as well as an optional 
        /// normalization factor. This adapter takes the first value in the frame, e.g. left in stereo audio.
        /// 
        /// @param channels is the number of values per frame.
        /// @param normalize is a divisor for each sample as it's pushed into the destination. 
        StaticAudioAdapter(size_t channels, float normalize = 1.0)
            : channels(channels)
            , normalize(normalize)
        {}

        virtual void Write(Ring<float>& destination, const void* source, size_t count)
        {
            const T* typed = reinterpret_cast<const T*>(source);

            // In the case that the new data is longer than the window, start where there's only one window left
            if (count > destination.Size())
            {
                typed += (count - destination.Size()) * this->channels;
                count = destination.Size();
            }

            for (size_t i = 0; i < count; ++i)
            {
                destination.Write(static_cast<float>(typed[i * this->channels]) / this->normalize);
            }
        }

    private:
        size_t channels;
        float normalize;
    };

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
        virtual void Handle(const void* data, size_t count, DWORD flags);

        /// Run the fftwf_plan on the data buffer we've been adding to in AudioAnalyzer::Handle.
        void Analyze();

        /// Get a reference to the spectrum data.
        /// 
        /// @returns a const reference to the complex spectrum output.
        const std::vector<FFTWFComplex>& Spectrum() const;

    protected:
        /// The number of audio frames to use for the FFT.
        size_t window{ 0 };

        /// Audio adapter
        std::unique_ptr<AudioAdapter> adapter;

        /// Initial buffer for the real samples that we'll run the FFT on.
        Ring<float> buffer;

        /// Precomputed FFT parameters and allocated memory.
        FFTWFPlan fft;

        /// A container for the result of the FFT.
        std::vector<FFTWFComplex> spectrum;
    };
}
