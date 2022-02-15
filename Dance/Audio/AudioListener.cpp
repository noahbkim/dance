#include "AudioListener.h"

AudioListener::AudioListener() {}

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

    TRACE("buffer size: " << this->bufferSize << ", rate: " << this->waveFormat->nSamplesPerSec);
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

    return S_OK;
}
