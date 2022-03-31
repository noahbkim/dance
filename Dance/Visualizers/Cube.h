#pragma once

#include "Graphics/Renderable.h"
#include "Graphics/Buffer.h"
#include "Graphics/Primitive.h"
#include "Graphics/Shader.h"
#include "Graphics/Camera.h"
#include "Mathematics/Mathematics.h"
#include "Visualizer.h"

using Vertex = SimpleVertex::Structure;

const static Vertex VERTICES[] = {
	{ Vector3F(-0.5f, -0.5f, -0.5f), Vector3F(0, 0, -1.0f), Color4F(0.0f, 1.0f, 1.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(-0.5f, 0.5f, -0.5f), Vector3F(0, 0, -1.0f), Color4F(0.0f, 1.0f, 1.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, -0.5f), Vector3F(0, 0, -1.0f), Color4F(0.0f, 1.0f, 1.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(0.5f, -0.5f, -0.5f), Vector3F(0, 0, -1.0f), Color4F(0.0f, 1.0f, 1.0f, 1.0f), { 1.0f, 0.0f } },

	{ Vector3F(-0.5f, -0.5f, 0.5f), Vector3F(0, 0, 1.0f), Color4F(1.0f, 0.0f, 0.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(-0.5f, 0.5f, 0.5f), Vector3F(0, 0, 1.0f), Color4F(1.0f, 0.0f, 0.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, 0.5f), Vector3F(0, 0, 1.0f), Color4F(1.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(0.5f, -0.5f, 0.5f), Vector3F(0, 0, 1.0f), Color4F(1.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0.0f } },

	{ Vector3F(-0.5f, 0.5f, -0.5f), Vector3F(0, 1.0f, 0), Color4F(1.0f, 0.0f, 1.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(-0.5f, 0.5f, 0.5f), Vector3F(0, 1.0f, 0), Color4F(1.0f, 0.0f, 1.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, 0.5f), Vector3F(0, 1.0f, 0), Color4F(1.0f, 0.0f, 1.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, -0.5f), Vector3F(0, 1.0f, 0), Color4F(1.0f, 0.0f, 1.0f, 1.0f), { 1.0f, 0.0f } },

	{ Vector3F(-0.5f, -0.5f, -0.5f), Vector3F(0, -1.0f, 0), Color4F(0.0f, 1.0f, 0.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(-0.5f, -0.5f, 0.5f), Vector3F(0, -1.0f, 0), Color4F(0.0f, 1.0f, 0.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(0.5f, -0.5f, 0.5f), Vector3F(0, -1.0f, 0), Color4F(0.0f, 1.0f, 0.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(0.5f, -0.5f, -0.5f), Vector3F(0, -1.0f, 0), Color4F(0.0f, 1.0f, 0.0f, 1.0f), { 1.0f, 0.0f } },

	{ Vector3F(-0.5f, -0.5f, -0.5f), Vector3F(-1.0f, 0, 0), Color4F(0.0f, 0.0f, 1.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(-0.5f, -0.5f, 0.5f), Vector3F(-1.0f, 0, 0), Color4F(0.0f, 0.0f, 1.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(-0.5f, 0.5f, 0.5f), Vector3F(-1.0f, 0, 0), Color4F(0.0f, 0.0f, 1.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(-0.5f, 0.5f, -0.5f), Vector3F(-1.0f, 0, 0), Color4F(0.0f, 0.0f, 1.0f, 1.0f), { 1.0f, 0.0f } },

	{ Vector3F(0.5f, -0.5f, -0.5f), Vector3F(1.0f, 0, 0), Color4F(1.0f, 1.0f, 0.0f, 1.0f), { 0.0f, 0.0f } },
	{ Vector3F(0.5f, -0.5f, 0.5f), Vector3F(1.0f, 0, 0), Color4F(1.0f, 1.0f, 0.0f, 1.0f), { 0.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, 0.5f), Vector3F(1.0f, 0, 0), Color4F(1.0f, 1.0f, 0.0f, 1.0f), { 1.0f, 1.0f } },
	{ Vector3F(0.5f, 0.5f, -0.5f), Vector3F(1.0f, 0, 0), Color4F(1.0f, 1.0f, 0.0f, 1.0f), { 1.0f, 0.0f } },
};

const static uint16_t INDICES[] = {
	2 , 0 , 1 , 3 , 0 , 2 ,
	6 , 5 , 4 , 7 , 6 , 4 ,
	10, 8 , 9 , 11, 8 , 10,
	14, 13, 12, 15, 14, 12,
	18, 16, 17, 19, 16, 18,
	22, 21, 20, 23, 22, 20,
};

class Cube : public Renderable
{
public:
	Cube() : Renderable() {}

	Cube(ComPtr<ID3D11Device> device) : Renderable(device) 
	{
		this->vertices = IndexedVertexBuffer(device, lengthof(VERTICES), VERTICES, lengthof(INDICES), INDICES);
		this->shader = Shader(device, L"Shader/Mesh.hlsl", SimpleVertex::LAYOUT, lengthof(SimpleVertex::LAYOUT));
	}
};

class CubeVisualizer : public ThreeVisualizer, public AudioVisualizer
{
public:
	CubeVisualizer(const Visualizer::Dependencies& dependencies) 
		: size{}
		, ThreeVisualizer(dependencies)
		, AudioVisualizer(dependencies)
	{
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->cube = Cube(this->d3dDevice);
		this->camera = Camera(this->d3dDevice, Matrix4F(), Matrix4F());
		this->theta = 0.0f;

		RECT size;
		::GetClientRect(dependencies.Window, &size);
		this->Resize(size);
	}

	virtual HRESULT Unsize()
	{
		OK(ThreeVisualizer::Unsize());
		return S_OK;
	}

	virtual HRESULT Resize(const RECT& size)
	{
		this->size = size;
		OK(ThreeVisualizer::Resize(size));
		OK(this->SetViewport(size));
		OK(this->SetProjection(size));
		return S_OK;
	}

	virtual void Render()
	{
		const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		this->d3dDeviceContext->ClearRenderTargetView(
			this->d3dBackBufferView.Get(), 
			color);

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
		this->dxgiSwapChain->Present(1, 0);
	}

	virtual void Update(float delta)
	{
		AudioVisualizer::Update(delta);

		const size_t normalize = this->analyzer.Spectrum().size();

		FLOAT level = 0.0f;
		for (size_t i = 100; i < 600; ++i)
		{
			level += this->analyzer.Spectrum()[i].magnitude(normalize);
		}

		this->theta += delta;
		this->cube.Transform() = Matrix4F::Scale(1.0f)
			* Matrix4F::YRotation(this->theta)
			* Matrix4F::XRotation(0.45f * this->theta)
			* Matrix4F::ZRotation(0.85f * this->theta);
	}

protected:
	RECT size;
	Cube cube;
	Camera camera;
	float theta;

	virtual HRESULT SetViewport(const RECT& size)
	{
		D3D11_VIEWPORT viewport
		{
			0.0f,
			0.0f,
			(FLOAT)(size.right - size.left),
			(FLOAT)(size.bottom - size.top),
			0.0f,
			1.0f
		};
		this->d3dDeviceContext->RSSetViewports(1, &viewport);
		return S_OK;
	}

	virtual HRESULT SetProjection(const RECT& size)
	{
		this->camera.Projection = Matrix4F::YRotation(-Geometry::PiOver2)
			* Matrix4F::ZRotation(-Geometry::PiOver2)
			* Matrix4F::Perspective(
				Geometry::ToRadians(70.0f),
				(FLOAT)(size.right - size.left),
				(FLOAT)(size.bottom - size.top),
				25.0f,
				10000.0f);
		this->camera.WorldToCamera = Matrix4F::Translation(Vector3(500.0f, 0.0f, 0.0f));
		return S_OK;
	}
};
