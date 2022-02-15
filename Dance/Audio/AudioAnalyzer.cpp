#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer() : AudioListener() {}

AudioAnalyzer::AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration)
    : AudioListener(device, duration)
    , fftwPlan()
{
    // The window is how much data we will ever analyze at once; we calculate it from duration
    this->window = static_cast<UINT32>(duration * this->waveFormat->nSamplesPerSec / ONE_SECOND);
    TRACE("window: " << this->window);

    // Resize the data container to fit our window continuously at any index
    this->buffer.resize(this->window);
    this->fftwBuffer.resize(this->window);
}

void AudioAnalyzer::Handle(const PCMAudioFrame* data, UINT32 count, DWORD flags)
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
        this->buffer[(this->index + i) % this->window].real = static_cast<float>(data[i].left);
    }

    // Update index, count, and timestamp
    this->index = (this->index + count) % this->window;
    this->count = std::min(this->count + count, this->window);
    // TRACE("index: " << this->index << ", count: " << this->count << ", time: " << this->timestamp.QuadPart);
}

void AudioAnalyzer::Analyze()
{
//    auto end = std::copy(this->buffer.begin() + this->index, this->buffer.end(), this->fftwBuffer.begin());
//    std::copy(this->buffer.begin(), this->buffer.begin() + index, end);
    this->fftwPlan.Execute();
}

void AudioAnalyzer::Sink(fftwf_complex* out)
{
    this->fftwPlan.Overwrite(::fftwf_plan_dft_1d(
        this->window,
        reinterpret_cast<fftwf_complex*>(this->buffer.data()),
        out,
        FFTW_FORWARD,
        FFTW_MEASURE));
}
