#include "Visualizer.h"

Visualizer::Visualizer
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: TransparentWindow(instance, windowClassName, windowTitle)
	, Runtime()
	, levels(5)
{
	this->levels.at(0) = { 0, 1 };
	this->levels.at(1) = { 0.5, 1 };
	this->levels.at(2) = { 0.75, 1 };
	this->levels.at(3) = { 0.25, 1 };
	this->levels.at(4) = { 0.1, 1 };
}

LRESULT CALLBACK Visualizer::Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCALCSIZE:
		return this->CalculateSize(wParam, lParam);
	case WM_NCHITTEST:
		return this->HitTest(lParam);
	case WM_ENTERSIZEMOVE:
		return this->StartResizeMove();
	case WM_EXITSIZEMOVE:
		return this->FinishResizeMove();
	case WM_PAINT:
		return this->Render();
	case WM_DESTROY:
		return this->Close();
	default:
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}
}

inline void Visualizer::RenderBorder()
{
	auto context = this->d2dDeviceContext;

	static const FLOAT THICKNESS = 5.0f;

	ComPtr<ID2D1SolidColorBrush> brush;
	D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.75f);
	context->CreateSolidColorBrush(color, brush.GetAddressOf());

	const FLOAT w = this->size.right - this->size.left;
	const FLOAT h = this->size.bottom - this->size.top;

	D2D1_RECT_F stroke{ 0, 0, w, THICKNESS };
	context->FillRectangle(stroke, brush.Get());

	stroke = { 0, h - THICKNESS, w, h };
	context->FillRectangle(stroke, brush.Get());

	stroke = { 0, THICKNESS, THICKNESS, h - THICKNESS };
	context->FillRectangle(stroke, brush.Get());

	stroke = { w - THICKNESS, THICKNESS, w, h - THICKNESS };
	context->FillRectangle(stroke, brush.Get());
}

LRESULT Visualizer::Render()
{
	// Set target, begin, clear
	auto context = this->d2dDeviceContext;
	context->SetTarget(this->d2dBitmap.Get());
	context->BeginDraw();
	context->Clear();

	this->RenderBorder();

	// End and present
	context->EndDraw();
	this->dxgiSwapChain->Present(1, 0);
	return 0;
}

void Visualizer::Update(double delta)
{
	
}

LRESULT Visualizer::Close()
{
	this->Destroy();
	::PostQuitMessage(0);
	return 0;
}
