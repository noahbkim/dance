#pragma once

#include "Common.h"

/// A REFERENCE_TIME increment is 100 nanoseconds.
#define ONE_SECOND 10000000
#define ONE_MILLISECOND 10000

namespace Dance::Audio
{
    /// A PCM audio frame is a pair of signed 16 - bit integers representing left and right.
    struct PCMAudioFrame
    {
        INT16 left;
        INT16 right;
    };

    /// Listens to system audio and writes to a ring buffer.
    class AudioListener
    {
    public:
        /// Instantiate an empty audio listener. Does no allocation with no arguments but is consequently unusable. Will
        /// undoubtedly throw a nullptr exception if you try to use it in this state.
        AudioListener();

        /// Instantiate an audio listener with a device and buffer duration. Creates an audio client and audio capture
        /// client. Determines the default wave format and attempts to coerce it into 16-bit signed frames with two 
        /// channels. Preallocates the buffer to the corresponding size using the provided duration and sampling frequency
        /// of the audio device.
        /// 
        /// @param device expects a ComPtr to a system audio device.
        /// @param duration is a duration in 100 ns intervals corresponding to hnsPeriodicity in IAudioClient::Initialize.
        /// @exception ComError if format determination fails, audio client initialization fails, or capture setup fails.
        AudioListener(ComPtr<IMMDevice> device, REFERENCE_TIME duration);

        /// Enables the listener by starting the audio client.
        /// 
        /// @returns the result of starting the audio client.
        virtual HRESULT Enable();

        /// Iterates through any newly available audio packets and invokes the handler on each one.
        /// 
        /// @exception ComError if any capture client operations fail.
        /// @returns whether any new packets were received by the listener and passed to handle.
        /// @seealso https://docs.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer
        virtual bool Listen();

        /// Disables the listener by stopping the audio client.
        /// 
        /// @returns the result of stopping the audio client.
        virtual HRESULT Disable();

    protected:
        ComPtr<IMMDevice> mmDevice = nullptr;
        ComPtr<IAudioClient> audioClient = nullptr;
        ComPtr<IAudioCaptureClient> audioCaptureClient = nullptr;

        /// Wave format struct populated by AudioListener::DetermineWaveFormat
        std::unique_ptr<WAVEFORMATEX, CoTaskDeleter<WAVEFORMATEX>> waveFormat = nullptr;

        /// The total size of the audio client buffer, which is at least as large as we request via duration.
        UINT32 bufferSize{ 0 };

        /// Virtual handler for new audio packets from the bound audio device captured by the audio client. This method is
        /// invoked by AudioListener::Listen.
        /// 
        /// @param data is a pointer to the available audio capture packet.
        /// @param count is the number of frames in the packet.
        /// @param flags contains information about discontinuities, silence, etc.
        /// @see AudioListener::Listen
        virtual void Handle(const void* data, size_t count, DWORD flags) = 0;

    private:
        /// Whether the listener is enabled.
        bool listening = false;
    };
}
