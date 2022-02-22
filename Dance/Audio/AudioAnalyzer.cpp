#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer() : AudioListener() {}

AudioAnalyzer::AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration)
    : AudioListener(device, duration)
    , fft()
{
    // The window is how much data we will ever analyze at once; we calculate it from duration
    this->window = static_cast<size_t>(duration) * this->waveFormat->nSamplesPerSec / ONE_SECOND;

    // Resize the data container to fit our window continuously at any index
    this->buffer.resize(this->window);
    this->spectrum.resize(this->window / 2 + 1);
#ifdef ORDER
    this->ordered.resize(this->window);
#endif

    // Create the FFT plan
    this->fft.Overwrite(::fftwf_plan_dft_r2c_1d(
        this->window,
#ifdef ORDER
        reinterpret_cast<float*>(this->ordered.data()),
#else
        reinterpret_cast<float*>(this->buffer.data()),
#endif
        reinterpret_cast<fftwf_complex*>(this->spectrum.data()),
        static_cast<unsigned int>(FFTW_MEASURE)));
}

void AudioAnalyzer::Handle(const PCMAudioFrame* data, size_t count, DWORD flags)
{
    // https://stackoverflow.com/questions/64158704/wasapi-captured-packets-do-not-align
    if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
    {
        this->count = 0;
        TRACE("discontinuity!");
    }
    if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
    {
        TRACE("silent!");
    }

    // In the case that the new data is longer than the window, start where there's only one window left
    if (count > this->window)
    {
        data += count - this->window;
        count = this->window;
    }

    // Write as ring
    for (size_t i = 0; i < count; ++i)
    {
        this->buffer[(this->index + i) % this->window] = static_cast<float>(data[i].left);
    }

    // Update index and count
    this->index = (this->index + count) % this->window;
    this->count = std::min(this->count + count, this->window);
}

void AudioAnalyzer::Analyze()
{
#ifdef ORDER
    auto end = std::copy(this->buffer.begin() + this->index, this->buffer.end(), this->ordered.begin());
    std::copy(this->buffer.begin(), this->buffer.begin() + index, end);
#endif
    this->fft.Execute();
}

const std::vector<FFTWFComplex>& AudioAnalyzer::Spectrum() const 
{
    return this->spectrum;
}
