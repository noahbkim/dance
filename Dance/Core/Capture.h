#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

#include <exception>
#include <algorithm>

#pragma comment(lib, "Winmm.lib")

// A REFERENCE_TIME increment is 100 nanoseconds
#define ONE_SECOND (1e9 / 1e2)
#define ONE_MILLISECOND (1e9 / 1e3 / 1e2)

ComPtr<IMMDevice> getDefaultDevice();
std::wstring getDeviceId(ComPtr<IMMDevice> device);
std::wstring getDeviceFriendlyName(ComPtr<IMMDevice> device);

class Capture
{
public:
    ComPtr<IMMDevice> Device;
    ComPtr<IAudioClient> AudioClient = nullptr;
    ComPtr<IAudioCaptureClient> AudioCaptureClient = nullptr;
    INT16* Buffer = nullptr;
    size_t BufferSize = 0;

    Capture() : Device(nullptr) {}
    Capture(ComPtr<IMMDevice> device) : Device(device) {}

    ~Capture()
    {
        CoTaskMemFree(this->waveFormat);
        delete[] this->Buffer;
    }

    HRESULT Setup()
    {
        OK(this->Device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(this->AudioClient.ReleaseAndGetAddressOf())));
        OK(this->DetermineWaveFormat());

        OK(this->AudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            ONE_SECOND,
            0,
            this->waveFormat,
            nullptr));

        OK(this->AudioClient->GetBufferSize(&this->bufferFrameCount));
        this->bufferDuration = (double)ONE_SECOND * this->bufferFrameCount / this->waveFormat->nSamplesPerSec;
        OK(this->AudioClient->GetService(
            __uuidof(IAudioCaptureClient),
            reinterpret_cast<void**>(this->AudioCaptureClient.ReleaseAndGetAddressOf())));

        this->BufferSize = static_cast<size_t>(this->bufferFrameCount) * this->waveFormat->nChannels;
        this->Buffer = new INT16[this->BufferSize];
        
        return S_OK;
    }

    HRESULT Start()
    {
        return S_OK;
    }

    // https://stackoverflow.com/questions/64158704/wasapi-captured-packets-do-not-align

    HRESULT Sample()
    {
        BYTE* data;
        DWORD flags;
        UINT32 availableBufferFrameCount;
        UINT32 packetLength;

        OK(this->AudioCaptureClient->GetNextPacketSize(&packetLength));
        while (packetLength != 0)
        {
            OK(this->AudioCaptureClient->GetBuffer(
                &data,
                &availableBufferFrameCount,
                &flags, 
                nullptr, 
                nullptr));

            if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
            {
                TRACE("discontinuity!");
            }
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                TRACE("silent!");
            }

            size_t remainingBufferScalarCount = (size_t)availableBufferFrameCount * this->waveFormat->nChannels;
            while (remainingBufferScalarCount > 0)
            {
                size_t chunkSize = std::min(remainingBufferScalarCount, this->BufferSize - this->index);
                memcpy(this->Buffer + this->index, data, chunkSize);
                this->index = (this->index + chunkSize) % this->BufferSize;
                remainingBufferScalarCount -= chunkSize;
            }

            OK(this->AudioCaptureClient->ReleaseBuffer(availableBufferFrameCount));
            OK(this->AudioCaptureClient->GetNextPacketSize(&packetLength));
        }
    }

    void Kill()
    {
        this->alive = false;
    }

    void Main()
    {
        OKE(this->AudioClient->Start());

        TRACE("bufferDuration: " << this->bufferDuration);
        TRACE("bufferFrameCount: " << this->bufferFrameCount);

        this->alive = true;
        while (this->alive)
        {
            Sleep(this->bufferDuration / ONE_MILLISECOND / 2);
            this->Sample();
        }

        TRACE("exiting capture!");
        OKE(this->AudioClient->Stop());
    }

    static DWORD WINAPI Main(LPVOID lpParam)
    {
        Capture* capture = reinterpret_cast<Capture*>(lpParam);
        capture->Main();
        return 0;
    }

private:
    size_t index = 0;
    WAVEFORMATEX* waveFormat = nullptr;
    REFERENCE_TIME bufferDuration = 0;
    UINT32 bufferFrameCount = 0;
    bool alive = false;

    HRESULT DetermineWaveFormat()
    {
        OK(this->AudioClient->GetMixFormat(&this->waveFormat));

        // Switch down to 2 channels
        if (this->waveFormat->nChannels > 2)
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
            WAVEFORMATEXTENSIBLE* waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(this->waveFormat);
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
        OK(this->AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, this->waveFormat, NULL));
    }
};
