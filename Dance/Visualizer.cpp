#include "Visualizer.h"
#include "Engine/D3D/Camera.h"
#include "Mathematics.h"
#include "FFTW3/fftw3.h"

#pragma comment(lib, "FFTW3/libfftw3f-3.lib")

static const int MENU_EXIT = 42;

Visualizer::Visualizer
(
	InstanceHandle instance,
	std::wstring windowClassName,
	std::wstring windowTitle
)
	: TransparentWindow3D(instance, windowClassName, windowTitle)
	, Runtime()
	, levels(5)
{
	this->levels.at(0) = { 0, 1 };
	this->levels.at(1) = { 0.5, 1 };
	this->levels.at(2) = { 0.75, 1 };
	this->levels.at(3) = { 0.25, 1 };
	this->levels.at(4) = { 0.1, 1 };
}

HRESULT Visualizer::Create()
{
	ComPtr<IMMDevice> device = getDefaultDevice();
	TRACE("capturing " << getDeviceFriendlyName(device.Get()));
	this->capture = Capture(device);
	this->captureThread = CreateThread(nullptr, 0, Capture::Main, &this->capture, 0, nullptr);

	TransparentWindow3D::Create();

	/*
	this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetClientRect(this->window.get(), &this->size);
	Matrix4F projection = Matrix4F::YRotation(-Geometry::PiOver2)
		* Matrix4F::ZRotation(-Geometry::PiOver2)
		* Matrix4F::Perspective(
			Geometry::ToRadians(70.0f),
			(FLOAT)(this->size.right - this->size.left),
			(FLOAT)(this->size.bottom - this->size.top),
			25.0f,
			10000.0f);
	Matrix4F worldToCamera = Matrix4F::Translation(Vector3(500.0f, 0.0f, 0.0f));

	this->cube = Cube(this->d3dDevice);
	this->camera = Camera(this->d3dDevice, worldToCamera, projection);
	this->theta = 0.0f;
	*/

	return S_OK;
}

HRESULT Visualizer::Resize()
{
	OK(TransparentWindow3D::Resize());

	/*
	D3D11_VIEWPORT viewport
	{
		0.0f, 
		0.0f,
		(FLOAT)(this->size.right - this->size.left), 
		(FLOAT)(this->size.bottom - this->size.top),
		0.0f, 
		1.0f 
	};
	this->d3dDeviceContext->RSSetViewports(1, &viewport);

	this->camera.Projection = Matrix4F::YRotation(-Geometry::PiOver2)
		* Matrix4F::ZRotation(-Geometry::PiOver2)
		* Matrix4F::Perspective(
			Geometry::ToRadians(70.0f),
			(FLOAT)(this->size.right - this->size.left),
			(FLOAT)(this->size.bottom - this->size.top),
			25.0f,
			10000.0f);
	*/

	return S_OK;
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
	case WM_MOUSEHOVER:
	case WM_NCMOUSEHOVER:
		return this->MouseHover(wParam, lParam);
	case WM_MOUSELEAVE:
	case WM_NCMOUSELEAVE:
		return this->MouseLeave(wParam, lParam);
	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		return this->MouseMove(wParam, lParam);
	case WM_RBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		return this->RightButtonDown(wParam, lParam);
	case WM_COMMAND:
		return this->Command(wParam, lParam);
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

	ComPtr<ID2D1SolidColorBrush> brush;
	D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.5f);
	context->CreateSolidColorBrush(color, brush.GetAddressOf());

	static const FLOAT THICKNESS = 2.0f;
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

	if (this->mouseHovering || this->isResizingOrMoving)
	{
		this->RenderBorder();
	}

	ComPtr<ID2D1SolidColorBrush> brush;
	D2D1_COLOR_F const color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	context->CreateSolidColorBrush(color, brush.GetAddressOf());
	const FLOAT w = this->size.right - this->size.left;
	const FLOAT h = this->size.bottom - this->size.top;

	D2D1_RECT_F stroke;
	const FLOAT b = 4;
	const FLOAT u = w / ((b + 1) * this->levels.size() + 1);

	for (size_t i = 0; i < this->levels.size(); ++i)
	{
		const FLOAT left = u * ((b + 1) * i + 1);
		stroke = {
			std::round(left),
			h * (1.0f - this->levels[i].level),
			std::round(left + b * u),
			h
		};
		context->FillRectangle(stroke, brush.Get());
	}

	// End and present
	context->EndDraw();

	/*
	this->d3dDeviceContext->ClearDepthStencilView(
		this->d3dDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0);
	this->d3dDeviceContext->OMSetRenderTargets(
		1, 
		this->d3dBackBufferView.GetAddressOf(),
		this->d3dDepthStencilView.Get());

	this->camera.Activate();
	this->cube.Render();
	*/
	
	this->dxgiSwapChain->Present(1, 0);
	return 0;
}

void Visualizer::Update(double delta)
{
	for (Bar& bar : this->levels)
	{
		bar.level += delta * bar.direction / 2;
		if (bar.level >= 1.0)
		{
			bar.level = 1.0;
			bar.direction = -1;
		}
		else if (bar.level <= 0.0)
		{
			bar.level = 0.0;
			bar.direction = 1;
		}
	}

	/*
	this->theta += delta;
	this->cube.Transform() = Matrix4F::Scale(200.0f) 
		* Matrix4F::YRotation(this->theta) 
		* Matrix4F::XRotation(0.45f * this->theta)
		* Matrix4F::ZRotation(0.85f * this->theta);
	*/
}

LRESULT Visualizer::MouseMove(WPARAM wParam, LPARAM lParam)
{
	this->mouseHovering = true;
	if (!this->mouseTracking)
	{
		TRACKMOUSEEVENT tracking{};
		tracking.cbSize = sizeof(tracking);
		tracking.dwFlags = TME_NONCLIENT | TME_LEAVE;
		tracking.hwndTrack = this->window.get();
		tracking.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tracking);
		this->mouseTracking = true;
	}
	return 0;
}

LRESULT Visualizer::MouseHover(WPARAM wParam, LPARAM lParam)
{
	this->mouseHovering = true;
	return 0;
}

LRESULT Visualizer::MouseLeave(WPARAM wParam, LPARAM lParam)
{
	this->mouseHovering = false;
	this->mouseTracking = false;
	return 0;
}

LRESULT Visualizer::RightButtonDown(WPARAM wParam, LPARAM lParam)
{
	HMENU hPopupMenu = ::CreatePopupMenu();
	BET(InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MENU_EXIT, L"Exit"));
	BET(::SetForegroundWindow(this->window.get()));
	POINT point{ LOWORD(lParam), HIWORD(lParam) };
	BET(::TrackPopupMenu(
		hPopupMenu, 
		TPM_TOPALIGN | TPM_LEFTALIGN,
		point.x,
		point.y,
		0,
		this->window.get(),
		NULL));
}

LRESULT Visualizer::Command(WPARAM wParam, LPARAM lParam)
{
	// If from menu
	if (HIWORD(wParam) == 0)
	{
		switch (LOWORD(wParam))
		{
		case MENU_EXIT:
			::DestroyWindow(this->window.get());
			return 0;
		}
	}

	return 0;
}

LRESULT Visualizer::Close()
{
	this->capture.Kill();
	WaitForSingleObject(this->captureThread, INFINITE);

	this->Destroy();
	::PostQuitMessage(0);
	return 0;
}
