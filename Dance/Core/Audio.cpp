#include "Audio.h"

ComPtr<IMMDevice> getDefaultDevice()
{
    ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    OKE(CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)deviceEnumerator.ReleaseAndGetAddressOf()));

    ComPtr<IMMDevice> device;
    OKE(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.ReleaseAndGetAddressOf()));
    return device;
}

std::wstring getDeviceId(ComPtr<IMMDevice> device)
{
    LPWSTR id = NULL;
    OKE(device->GetId(&id));
    std::wstring result(id);

    ::CoTaskMemFree(id);
    return result;
}

std::wstring getDeviceFriendlyName(ComPtr<IMMDevice> device)
{
    ComPtr<IPropertyStore> propertyStore;
    OKE(device->OpenPropertyStore(STGM_READ, propertyStore.ReleaseAndGetAddressOf()));

    PROPVARIANT name;
    ::PropVariantInit(&name);
    OKE(propertyStore->GetValue(PKEY_Device_FriendlyName, &name));
    std::wstring result(name.pwszVal);
    ::PropVariantClear(&name);

    return result;
}

AudioListener::AudioListener() : mmDevice(nullptr) {}

AudioListener::AudioListener(ComPtr<IMMDevice> device, REFERENCE_TIME duration) : mmDevice(device)
{
    // Active the multimedia device and get an audio client
    OKE(this->mmDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(this->audioClient.ReleaseAndGetAddressOf())));

    // Determine the available wave format
    OKE(this->DetermineWaveFormat());

    // Initialize the audio client and request the provided duration and determined wave format
    OKE(this->audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        duration,
        0,
        this->waveFormat.get(),
        nullptr));

    // Ask for the true buffer size, as it could be larger
    OKE(this->audioClient->GetBufferSize(&this->bufferSize));

    // Get the capture client so we can start and stop capture
    OKE(this->audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        reinterpret_cast<void**>(this->audioCaptureClient.ReleaseAndGetAddressOf())));

    // Calculate the true buffer duration
    this->bufferDuration = (double)ONE_SECOND * this->bufferSize / this->waveFormat->nSamplesPerSec;

    TRACE("buffer size: " << this->bufferSize << ", buffer duration: " << this->bufferDuration << ", rate: " << this->waveFormat->nSamplesPerSec);
}

bool AudioListener::Listen()
{
    // https://docs.microsoft.com/en-us/windows/win32/coreaudio/capturing-a-stream
    PCMAudioFrame* data;
    DWORD flags;
    UINT32 count;

    // Loop over available packets
    OKE(this->audioCaptureClient->GetNextPacketSize(&count));
    bool available = count > 0;

    while (count != 0)
    {
        // This overewrites count with the real size of the buffer
        OKE(this->audioCaptureClient->GetBuffer(
            reinterpret_cast<BYTE**>(&data),
            &count,
            &flags,
            nullptr,
            nullptr));

        this->Handle(data, count, flags);

        // Release the data we just asked for
        OKE(this->audioCaptureClient->ReleaseBuffer(count));

        // Separately, ask for the new packet size in case there's another to process
        OKE(this->audioCaptureClient->GetNextPacketSize(&count));
    }

    return available;
}


HRESULT AudioListener::Enable()
{
    OK(this->audioClient->Start());
    return S_OK;
}

HRESULT AudioListener::Disable()
{
    OK(this->audioClient->Stop());
    return S_OK;
}

/*

    this->listening = true;
    while (this->listening)
    {
        ::Sleep(this->interval);
        this->Listen();
    }

void AudioListener::Stop()
{
    this->listening = false;
}
*/

HRESULT AudioListener::DetermineWaveFormat()
{
    // Quirk of std::unique_ptr is that it won't give you its pointer address
    WAVEFORMATEX* waveFormat;
    this->audioClient->GetMixFormat(&waveFormat);
    this->waveFormat.reset(waveFormat);

    // Switch down to 2 channels; we don't support one channel right now TODO
    if (this->waveFormat->nChannels != 2)
    {
        this->waveFormat->nChannels = 2;
    }

    // First convert from floating point to PCM
    if (this->waveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        this->waveFormat->wFormatTag = WAVE_FORMAT_PCM;
        this->waveFormat->wBitsPerSample = 16;
        this->waveFormat->nBlockAlign = (this->waveFormat->wBitsPerSample / 8) * this->waveFormat->nChannels;
        this->waveFormat->nAvgBytesPerSec = this->waveFormat->nSamplesPerSec * this->waveFormat->nBlockAlign;
    }
    else if (this->waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        WAVEFORMATEXTENSIBLE* waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(this->waveFormat.get());
        if (waveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
        {
            waveFormatExtensible->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            waveFormatExtensible->Format.wBitsPerSample = 16;
            waveFormatExtensible->Format.nBlockAlign = (waveFormatExtensible->Format.wBitsPerSample / 8) * waveFormatExtensible->Format.nChannels;
            waveFormatExtensible->Format.nAvgBytesPerSec = waveFormatExtensible->Format.nSamplesPerSec * waveFormatExtensible->Format.nBlockAlign;
            waveFormatExtensible->Samples.wValidBitsPerSample = 16;
        }
    }

    // Verify that this format is supported
    OK(this->audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, this->waveFormat.get(), NULL));
}

AudioAnalyzer::AudioAnalyzer() : AudioListener() {}

AudioAnalyzer::AudioAnalyzer(ComPtr<IMMDevice> device, REFERENCE_TIME duration) 
    : AudioListener(device, duration)
    , fftwPlan()
{
    // The window is how much data we will ever analyze at once; we calculate it from duration
    this->window = duration * this->waveFormat->nSamplesPerSec / ONE_SECOND;
    TRACE("window: " << this->window);

    // Resize the data container to fit our window continuously at any index
    this->buffer.resize(this->window);
    this->fftwBuffer.resize(this->window);
}

/*
AudioAnalyzer::~AudioAnalyzer()
{
    // ::fftwf_destroy_plan(this->plan);
}
*/

bool AudioAnalyzer::Listen()
{
    return AudioListener::Listen();
}

void AudioAnalyzer::Handle(PCMAudioFrame* data, UINT32 count, DWORD flags)
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
    auto end = std::copy(this->buffer.begin() + this->index, this->buffer.end(), this->fftwBuffer.begin());
    std::copy(this->buffer.begin(), this->buffer.begin() + index, end);
    this->fftwPlan.Execute();
}

void AudioAnalyzer::Sink(fftwf_complex* out)
{
    this->fftwPlan.Overwrite(::fftwf_plan_dft_1d(
        this->window,
        reinterpret_cast<fftwf_complex*>(this->fftwBuffer.data()),
        out,
        FFTW_FORWARD,
        FFTW_MEASURE));
}
