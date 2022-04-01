#pragma once

#include "Visualizer.h"
#include "Common.h"
#include "AudioAnalyzer.h"

namespace Dance::Audio
{
    class AudioVisualizer : public virtual Visualizer
    {
    public:
        AudioVisualizer(const Dependencies& dependencies);
        virtual ~AudioVisualizer();

        virtual void Update(double delta);

    protected:
        AudioAnalyzer analyzer;
    };
}
