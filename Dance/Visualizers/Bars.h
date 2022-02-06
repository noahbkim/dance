#pragma once

#include "Framework.h"
#include "Visualizer.h"

#include <array>
#include <algorithm>

#define SMOOTHING 10

class BarsVisualizer : TwoVisualizer, AudioVisualizer
{
public:
	BarsVisualizer() : size{} {}

	virtual HRESULT Create(const Visualizer::Dependencies& dependencies)
	{
		OK(TwoVisualizer::Create(dependencies));
		OK(AudioVisualizer::Create(dependencies));

		this->barCount = 30;
		this->sampleStart = 0;
		this->sampleEnd = this->spectrum.size() / 2 / 20;
		this->samplesPerBar = (this->sampleEnd - this->sampleStart) / this->barCount;
		this->levels.resize(this->barCount);

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
		OK(TwoVisualizer::Resize(size));
		return S_OK;
	}

	virtual void Render()
	{
		auto context = this->d2dDeviceContext;
		context->SetTarget(this->d2dBitmap.Get());
		context->BeginDraw();
		context->Clear();

		const FLOAT w = this->size.right - this->size.left;
		const FLOAT h = this->size.bottom - this->size.top;

		D2D1_RECT_F stroke;
		const FLOAT u = w / this->barCount;

		for (size_t i = 0; i < this->barCount; ++i)
		{
			FLOAT level = 0.0f;
			for (size_t j = this->samplesPerBar * i; j < this->samplesPerBar * (i + 1); ++j)
			{
				level += this->spectrum[j].magnitude(this->spectrum.size());
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
				h - level,
				std::round(left + u),
				h
			};
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
