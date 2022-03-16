#pragma once

#include "Framework.h"
#include "Visualizer.h"

#include <array>
#include <algorithm>

#define SMOOTHING 10

static inline float v(float n, float a, float h, float l)
{
	const float k = fmod(n + h / 30.0f, 12.0f);
	return l - a * std::max(std::min(k - 3.0f, 9.0f - k), -1.0f);
}

static inline D2D1::ColorF rgb(float h, float s, float l)
{
	const float a = s * std::min(l, 1.0f - l);
	return D2D1::ColorF(v(0.0f, a, h, l), v(8.0f, a, h, l), v(4.0f, a, h, l), 1.0f);
}

class BarsVisualizer : public TwoVisualizer, public AudioVisualizer
{
public:
	BarsVisualizer() : size{} {}

	virtual HRESULT Create(const Visualizer::Dependencies& dependencies)
	{
		OK(TwoVisualizer::Create(dependencies));
		OK(AudioVisualizer::Create(dependencies));

		// Bounds of actual FFT spectrum we use


		D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
		this->d2dDeviceContext->CreateSolidColorBrush(color, brush.GetAddressOf());

		return S_OK;
	}

	virtual HRESULT Unsize()
	{
		OK(TwoVisualizer::Unsize());
		return S_OK;
	}

	virtual HRESULT Resize(const RECT& size)
	{
		this->size = size;

		this->barCount = std::min(static_cast<size_t>(size.right - size.left) / 40, this->analyzer.Spectrum().size());
		this->sampleStart = 0;
		this->sampleEnd = std::max(this->barCount, this->analyzer.Spectrum().size() / 20);
		this->samplesPerBar = (this->sampleEnd - this->sampleStart) / this->barCount;
		this->levels.resize(this->barCount);

		OK(TwoVisualizer::Resize(size));
		return S_OK;
	}

	virtual void Render()
	{
		auto context = this->d2dDeviceContext;
		context->SetTarget(this->d2dBitmap.Get());
		context->BeginDraw();
		context->Clear();

		const FLOAT w = static_cast<float>(this->size.right - this->size.left);
		const FLOAT h = static_cast<float>(this->size.bottom - this->size.top);

		D2D1_RECT_F stroke;
		const FLOAT u = w / this->barCount;
		const size_t normalize = this->analyzer.Spectrum().size();

		for (size_t i = 0; i < this->barCount; ++i)
		{
			FLOAT level = 0.0f;
			for (size_t j = this->samplesPerBar * i; j < this->samplesPerBar * (i + 1); ++j)
			{
				level += this->analyzer.Spectrum()[j].magnitude(normalize);
			}
			this->levels[i][this->levelIndex] = level / this->samplesPerBar;

			level = 0.0f;
			for (size_t j = 0; j < SMOOTHING; ++j)
			{
				level += this->levels[i][j];
			}
			level /= SMOOTHING;
			
			const FLOAT left = u * i;
			stroke = {
				std::round(left),
				h - ((level * 65.0f)) / ((level * 65.0f) + 1) * h,
				std::round(left + u),
				h
			};


			brush->SetColor(rgb(std::max(360.0f, std::log(level * 65535) * 100.0f), 1.0f, 0.5f));
			context->FillRectangle(stroke, brush.Get());
		}

		this->levelIndex = (this->levelIndex + 1) % SMOOTHING;

		// End and present
		context->EndDraw();
		this->dxgiSwapChain->Present(1, 0);
	}

	virtual void Update(double delta)
	{
		AudioVisualizer::Update(delta);
	}

	virtual HRESULT Destroy()
	{
		OK(TwoVisualizer::Destroy());
		OK(AudioVisualizer::Destroy());
		return S_OK;
	}

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
