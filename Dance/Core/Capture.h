#pragma once

#include "Framework.h"

#include <exception>
#include <algorithm>

#pragma comment(lib, "Winmm.lib")

/*
try
{
    Capture capture(getDefaultDevice());
    capture.Start();
    capture.Debug();
    capture.Stop();
}
catch (CaptureException& exception)
{
    TRACE("error setting up audio capture: " << exception.hresult);
    return exception.hresult;
}
*/


// A REFERENCE_TIME increment is 100 nanoseconds
#define ONE_SECOND (1e9 / 1e2)
#define ONE_MILLISECOND (1e9 / 1e3 / 1e2)

struct CaptureException : public std::exception
{
    HRESULT hresult;

    CaptureException(HRESULT hresult) : hresult(hresult) {}

    const char* what() const throw ()
    {
        return "caught invalid HRESULT while capturing audio!";
    }
};

#define OK(call) if (HRESULT result = call) { throw CaptureException(result); }
#define RELEASE(object) if (object != nullptr) { object->Release(); }

std::wstring getDeviceId(IMMDevice* device)
{
    LPWSTR id = NULL;
    OK(device->GetId(&id));
    std::wstring result(id);

    CoTaskMemFree(id);
    return result;
}

std::wstring getDeviceFriendlyName(IMMDevice* device)
{
    IPropertyStore* propertyStore;
    OK(device->OpenPropertyStore(STGM_READ, &propertyStore));

    PROPVARIANT name;
    PropVariantInit(&name);
    OK(propertyStore->GetValue(PKEY_Device_FriendlyName, &name));
    std::wstring result(name.pwszVal);

    PropVariantClear(&name);
    propertyStore->Release();

    return result;
}

class Capture
{
public:
    IMMDevice* Device;
    IAudioClient* AudioClient = nullptr;
    IAudioCaptureClient* AudioCaptureClient = nullptr;
    INT16* Buffer = nullptr;
    size_t BufferSize = 0;

    Capture() : Device(nullptr) {}
    Capture(IMMDevice* device) : Device(device) {}

    ~Capture()
    {
        this->Device->Release();
        CoTaskMemFree(this->waveFormat);
        RELEASE(this->AudioClient);
        RELEASE(this->AudioCaptureClient);
        delete[] this->Buffer;
    }

    void Setup()
    {
        OK(this->Device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            (void**)&this->AudioClient));

        this->DetermineWaveFormat();
        OK(this->AudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            ONE_SECOND,
            0,
            this->waveFormat,
            nullptr));

        OK(this->AudioClient->GetBufferSize(&this->bufferFrameCount));
        this->bufferDuration = (double)ONE_SECOND * this->bufferFrameCount / this->waveFormat->nSamplesPerSec;
        OK(this->AudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&this->AudioCaptureClient));

        this->BufferSize = this->bufferFrameCount * this->waveFormat->nChannels;
        this->Buffer = new INT16[this->BufferSize];
    }

    void Start()
    {
        OK(this->AudioClient->Start());
    }

    // https://stackoverflow.com/questions/64158704/wasapi-captured-packets-do-not-align

    void Sample()
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

    void Stop()
    {
        OK(this->AudioClient->Stop());
    }

    void Debug()
    {
        clock();
        TRACE("bufferDuration" << this->bufferDuration);
        TRACE("bufferFrameCount" << this->bufferFrameCount);

        while (clock() < 10 * CLOCKS_PER_SEC)
        {
            Sleep(this->bufferDuration / ONE_MILLISECOND / 2);
            this->Sample();
        }
    }

private:
    size_t index = 0;
    WAVEFORMATEX* waveFormat = nullptr;
    REFERENCE_TIME bufferDuration = 0;
    UINT32 bufferFrameCount = 0;

    void DetermineWaveFormat()
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

IMMDevice* getDefaultDevice()
{
    IMMDeviceEnumerator* deviceEnumerator;
    OK(CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&deviceEnumerator));

    IMMDevice* device;
    OK(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device));
    return device;
}
