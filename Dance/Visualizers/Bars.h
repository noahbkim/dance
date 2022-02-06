#pragma once

#include "Framework.h"
#include "Visualizer.h"

class BarsVisualizer : TwoVisualizer, AudioVisualizer
{
public:
	BarsVisualizer() : size{} {}

	virtual HRESULT Create(const Visualizer::Dependencies& dependencies)
	{
		OK(TwoVisualizer::Create(dependencies));
		OK(AudioVisualizer::Create(dependencies));
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

		/*
		if (this->mouseHovering || this->isResizingOrMoving)
		{
			Visualizer::Border(context, this->size);
		}
		*/

		ComPtr<ID2D1SolidColorBrush> brush;
		D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
		context->CreateSolidColorBrush(color, brush.GetAddressOf());
		const FLOAT w = this->size.right - this->size.left;
		const FLOAT h = this->size.bottom - this->size.top;

		const size_t N = 30;

		const size_t S = this->spectrum.size() / 2 / 20;
		const size_t Q = S / N;

		D2D1_RECT_F stroke;
		const FLOAT u = w / N;

		for (size_t i = 0; i < N; ++i)
		{
			const FLOAT left = u * i;
			FLOAT level = 0.0f;
			for (size_t j = Q * i; j < Q * (i + 1); ++j)
			{
				const FLOAT re = this->spectrum[j].real / this->spectrum.size();
				const FLOAT im = this->spectrum[j].imaginary / this->spectrum.size();
				level += std::sqrtf(re * re + im * im) / Q;
			}

			stroke = {
				std::round(left),
				h - 30 * std::log2f(level),
				std::round(left + u),
				h
			};
			context->FillRectangle(stroke, brush.Get());
		}

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
};
