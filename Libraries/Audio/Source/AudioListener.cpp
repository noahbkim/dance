#include "AudioListener.h"

namespace Dance::Audio
{
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
        WAVEFORMATEX* waveFormat;
        this->audioClient->GetMixFormat(&waveFormat);
        this->waveFormat.reset(waveFormat);

        // Force down to two channels
        this->waveFormat->nChannels = 2;
        this->waveFormat->nBlockAlign = this->waveFormat->wBitsPerSample / 8 * 2;
        this->waveFormat->nAvgBytesPerSec = this->waveFormat->nSamplesPerSec * waveFormat->nBlockAlign;

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

        while (count > 0)
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
}
