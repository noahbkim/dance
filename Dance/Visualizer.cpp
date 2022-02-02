#include "Visualizer.h"
#include "Engine/Common/Buffer.h"

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
	TransparentWindow3D::Create();

	// Setup shader
	ID3DBlob* vsBlob;
	{
		ID3DBlob* shaderCompileErrorsBlob;
		OK(::D3DCompileFromFile(L"Shader/Shader.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob));
		OK(this->d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader));
	}

	{
		ID3DBlob* psBlob;
		ID3DBlob* shaderCompileErrorsBlob;
		OK(::D3DCompileFromFile(L"Shader/Shader.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob));
		OK(this->d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader));
		psBlob->Release();
	}

	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		OK(d3dDevice->CreateInputLayout(
			inputElementDesc, 
			ARRAYSIZE(inputElementDesc), 
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(), 
			&inputLayout));
		vsBlob->Release();
	}

	{
		PositionColorVertex data[] = 
		{
			{ { 0.0f,  0.5f,  0.0f }, { 0.f, 1.f, 0.f, 0.5f } },
			{ { 0.5f,  -0.5f, 0.0f }, { 1.f, 0.f, 0.f, 0.5f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.f, 0.f, 1.f, 0.5f } }
		};
		this->vertices = VertexBuffer(this->d3dDevice.Get(), 3, data);
	}

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
	
	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(this->size.right - this->size.left), (FLOAT)(this->size.bottom - this->size.top), 0.0f, 1.0f };
	this->d3dDeviceContext->RSSetViewports(1, &viewport);
	this->d3dDeviceContext->OMSetRenderTargets(1, this->d3dBackBufferView.GetAddressOf(), nullptr);

	this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->d3dDeviceContext->IASetInputLayout(inputLayout);
	this->d3dDeviceContext->VSSetShader(vertexShader, nullptr, 0);
	this->d3dDeviceContext->PSSetShader(pixelShader, nullptr, 0);
	this->vertices.Set();
	this->vertices.Draw();
	
	this->dxgiSwapChain->Present(1, 0);
	return 0;
}

void Visualizer::Update(double delta)
{
	
}

LRESULT Visualizer::Close()
{
	this->inputLayout->Release();
	this->pixelShader->Release();
	this->vertexShader->Release();
	this->Destroy();
	::PostQuitMessage(0);
	return 0;
}