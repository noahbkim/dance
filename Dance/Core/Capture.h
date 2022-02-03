#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

#include <exception>
#include <algorithm>
#include <vector>

#pragma comment(lib, "Winmm.lib")

// A REFERENCE_TIME increment is 100 nanoseconds
#define ONE_SECOND (1e9 / 1e2)
#define ONE_MILLISECOND (1e9 / 1e3 / 1e2)

ComPtr<IMMDevice> getDefaultDevice();
std::wstring getDeviceId(ComPtr<IMMDevice> device);
std::wstring getDeviceFriendlyName(ComPtr<IMMDevice> device);

template<typename T>
class CaptureBuffer
{
public:
    CaptureBuffer() : buffer(0) {}
    CaptureBuffer(size_t capacity) : buffer(capacity) {}

    void Discontinuity()
    {
        this->count = 0;
    }

    void Write(const T* data, size_t count)
    {
        // In the case that the new data is longer than the buffer, start where there's only one buffer left
        if (count > this->buffer.size())
        {
            this->Discontinuity();
            data += count - this->buffer.size();
        }

        // Write up until end of buffer, memcpy size is in bytes
        size_t fill = this->buffer.size() - this->index;
        memcpy(&this->buffer.at(this->index), data, fill * sizeof(T));

        // If there's more, start again at zero, looping around
        if (count > fill)
        {
            size_t rest = count - fill;
            memcpy(&this->buffer.at(0), data + fill, rest * sizeof(T));
        }

        // Update index, count, and timestamp
        this->index = (this->index + count) % this->buffer.size();
        this->count = std::min(this->count + count, this->buffer.size());
        QueryPerformanceCounter(&this->timestamp);

        TRACE("index: " << this->index << ", count: " << this->count << ", time: " << this->timestamp.QuadPart);
    }

    const std::vector<T>& Buffer() const
    {
        return this->buffer;
    }

    const size_t& Index() const
    {
        return this->index;
    }

    const size_t& Count() const
    {
        return this->count;
    }

    const LARGE_INTEGER& Timestamp() const
    {
        return this->timestamp;
    }

private:
    std::vector<T> buffer;
    size_t index{ 0 };
    size_t count{ 0 };
    LARGE_INTEGER timestamp{ 0 };
};

class Capture
{
public:    
    Capture() : device(nullptr) {}

    Capture(ComPtr<IMMDevice> device) : device(device) 
    {
        OKE(this->device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(this->audioClient.ReleaseAndGetAddressOf())));
        OKE(this->DetermineWaveFormat());

        OKE(this->audioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            ONE_SECOND,
            0,
            this->waveFormat.get(),
            nullptr));

        OKE(this->audioClient->GetBufferSize(&this->bufferFrameCount));
        OKE(this->audioClient->GetService(
            __uuidof(IAudioCaptureClient),
            reinterpret_cast<void**>(this->audioCaptureClient.ReleaseAndGetAddressOf())));

        // Buffer will be interleaved
        this->buffer = CaptureBuffer<INT16>(static_cast<size_t>(this->bufferFrameCount) * this->waveFormat->nChannels);
        this->bufferDuration = (double)ONE_SECOND * this->bufferFrameCount / this->waveFormat->nSamplesPerSec;
    }

    HRESULT Sample()
    {
        BYTE* data;
        DWORD flags;
        UINT32 availableBufferFrameCount;
        UINT32 packetLength;

        OK(this->audioCaptureClient->GetNextPacketSize(&packetLength));
        while (packetLength != 0)
        {
            OK(this->audioCaptureClient->GetBuffer(
                &data,
                &availableBufferFrameCount,
                &flags, 
                nullptr, 
                nullptr));

            // https://stackoverflow.com/questions/64158704/wasapi-captured-packets-do-not-align
            if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
            {
                this->buffer.Discontinuity();
                TRACE("discontinuity!");
            }
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                TRACE("silent!");
            }

            this->buffer.Write(
                reinterpret_cast<INT16*>(data),
                availableBufferFrameCount * this->waveFormat->nChannels);

            OK(this->audioCaptureClient->ReleaseBuffer(availableBufferFrameCount));
            OK(this->audioCaptureClient->GetNextPacketSize(&packetLength));
        }
    }

    const CaptureBuffer<INT16>& Buffer() const
    {
        return this->buffer;
    }

    REFERENCE_TIME BufferDuration() const
    {
        return this->bufferDuration;
    }

    UINT32 BufferFrameCount() const
    {
        return this->bufferFrameCount;
    }

    void Main()
    {
        OKE(this->audioClient->Start());

        TRACE("bufferDuration: " << this->bufferDuration);
        TRACE("bufferFrameCount: " << this->bufferFrameCount);

        this->alive = true;
        while (this->alive)
        {
            Sleep(this->bufferDuration / ONE_MILLISECOND / 2);
            this->Sample();
        }

        TRACE("exiting capture!");
        OKE(this->audioClient->Stop());
    }

    void Kill()
    {
        this->alive = false;
    }

    // For threading
    static DWORD WINAPI Main(LPVOID lpParam)
    {
        Capture* capture = reinterpret_cast<Capture*>(lpParam);
        capture->Main();
        return 0;
    }

private:
    ComPtr<IMMDevice> device;
    ComPtr<IAudioClient> audioClient = nullptr;
    ComPtr<IAudioCaptureClient> audioCaptureClient = nullptr;

    CaptureBuffer<INT16> buffer;
    REFERENCE_TIME bufferDuration = 0;
    UINT32 bufferFrameCount = 0;

    std::unique_ptr<WAVEFORMATEX, CoTaskDeleter<WAVEFORMATEX>> waveFormat = nullptr;

    bool alive = false;

    HRESULT DetermineWaveFormat()
    {
        // Quirk of std::unique_ptr is that it won't give you its pointer address
        WAVEFORMATEX* waveFormat;
        this->audioClient->GetMixFormat(&waveFormat);
        this->waveFormat.reset(waveFormat);

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
};
