#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

// A REFERENCE_TIME increment is 100 nanoseconds
#define ONE_SECOND 10000000
#define ONE_MILLISECOND 10000

struct PCMAudioFrame
{
    INT16 left;
    INT16 right;
};

class AudioListener
{
public:
    AudioListener();
    AudioListener(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

    virtual HRESULT Enable();
    virtual bool Listen();
    virtual HRESULT Disable();

protected:
    ComPtr<IMMDevice> mmDevice;
    ComPtr<IAudioClient> audioClient = nullptr;
    ComPtr<IAudioCaptureClient> audioCaptureClient = nullptr;
    std::unique_ptr<WAVEFORMATEX, CoTaskDeleter<WAVEFORMATEX>> waveFormat = nullptr;

    UINT32 bufferSize{ 0 };

    virtual void Handle(PCMAudioFrame* data, UINT32 count, DWORD flags) = 0;

private:
    HRESULT DetermineWaveFormat();
    bool listening = false;
};
