#include "AudioAnalyzer.h"

namespace Dance::Audio
{
    AudioAnalyzer::AudioAnalyzer() : AudioListener() {}

    AudioAnalyzer::AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration)
        : AudioListener(device, duration)
        , fft()
    {
        // The window is how much data we will ever analyze at once; we calculate it from duration
        this->window = static_cast<size_t>(duration) * this->waveFormat->nSamplesPerSec / ONE_SECOND;

        // Resize the data container to fit our window continuously at any index
        this->buffer.Resize(this->window);
        this->spectrum.resize(this->window / 2 + 1);

        // Create the FFT plan
        this->fft.Overwrite(::fftwf_plan_dft_r2c_1d(
            this->window,
            reinterpret_cast<float*>(this->buffer.Data()),
            reinterpret_cast<fftwf_complex*>(this->spectrum.data()),
            static_cast<unsigned int>(FFTW_MEASURE)));

        // Determine what kind of audio adapter we need
        if (this->waveFormat->wFormatTag == WAVE_FORMAT_PCM)
        {
            if (this->waveFormat->wBitsPerSample == 16)
            {
                this->adapter = std::make_unique<StaticAudioAdapter<INT16>>(this->waveFormat->nChannels, INT16_MAX);
            }
            else if (this->waveFormat->wBitsPerSample == 32)
            {
                this->adapter = std::make_unique<StaticAudioAdapter<INT32>>(this->waveFormat->nChannels, INT32_MAX);
            }
            else
            {
                throw ComError(E_INVALIDARG, "unknown PCM audio format");
            }
        }
        else if (this->waveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            this->adapter = std::make_unique<StaticAudioAdapter<float>>(this->waveFormat->nChannels);
        }
        else if (this->waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            WAVEFORMATEXTENSIBLE* waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(this->waveFormat.get());
            if (waveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                this->adapter = std::make_unique<StaticAudioAdapter<float>>(this->waveFormat->nChannels);
            }
            else if (waveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
            {
                this->adapter = std::make_unique<StaticAudioAdapter<INT16>>(this->waveFormat->nChannels, INT16_MAX);
            }
            else
            {
                throw ComError(E_INVALIDARG, "unknown extensible audio format");
            }
        }
        else
        {
            throw ComError(E_INVALIDARG, "uknown audio format tag");
        }
    }

    void AudioAnalyzer::Handle(const void* data, size_t count, DWORD flags)
    {
        // https://stackoverflow.com/questions/64158704/wasapi-captured-packets-do-not-align
        if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
        {
            this->buffer.Reset();
            TRACE("discontinuity!");
        }
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
        {
            TRACE("silent!");
        }

        this->adapter->Write(this->buffer, data, count);
    }

    void AudioAnalyzer::Analyze()
    {
        this->fft.Execute();
    }

    const std::vector<FFTWFComplex>& AudioAnalyzer::Spectrum() const
    {
        return this->spectrum;
    }
}