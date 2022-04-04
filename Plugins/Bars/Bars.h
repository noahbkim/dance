#pragma once

#include <array>
#include <algorithm>
#include <string>
#include <cmath>

#include "AudioVisualizer.h"
#include "TwoVisualizer.h"

#define SMOOTHING 10

using Dance::API::Visualizer;
using Dance::Two::TwoVisualizer;
using Dance::Audio::AudioVisualizer;

static std::filesystem::path DllPath;

class BarsVisualizer : public TwoVisualizer, public AudioVisualizer
{
public:
	BarsVisualizer(const Visualizer::Dependencies& dependencies);

	virtual HRESULT Unsize();
	virtual HRESULT Resize(const RECT& size);

	virtual void Render();
	virtual void Update(double delta);

protected:
	RECT size;
	size_t barCount;
	size_t sampleStart;
	size_t sampleEnd;
	size_t samplesPerBar;

	std::vector<std::array<float, SMOOTHING>> levels;
	size_t levelIndex = 0;

	ComPtr<ID2D1SolidColorBrush> brush;
};
