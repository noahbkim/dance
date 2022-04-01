#include "AudioVisualizer.h"
#include "Audio.h"
#include "Device.h"

namespace Dance::Audio
{
    AudioVisualizer::AudioVisualizer(const Visualizer::Dependencies& dependencies)
    {
        ComPtr<IMMDevice> device = Dance::Audio::getDefaultAudioDevice();
        TRACE("capturing " << Dance::Audio::getAudioDeviceFriendlyName(device.Get()));
        this->analyzer = AudioAnalyzer(device, ONE_SECOND / 20);
        this->analyzer.Enable();
    }

    AudioVisualizer::~AudioVisualizer()
    {
        this->analyzer.Disable();
    }

    void AudioVisualizer::Update(double delta)
    {
        if (this->analyzer.Listen())
        {
            this->analyzer.Analyze();
        }
    }
}
