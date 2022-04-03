#pragma once

#include "Renderable.h"
#include "Buffer.h"
#include "Primitive.h"
#include "Shader.h"
#include "Camera.h"
#include "Mathematics.h"

#include "Visualizer.h"
#include "ThreeVisualizer.h"
#include "AudioVisualizer.h"

using Dance::API::Visualizer;
using namespace Dance::Three::Graphics;
using namespace Dance::Three::Math;

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

	Cube(ComPtr<ID3D11Device> device, const std::wstring& shaderPath) : Renderable(device)
	{
		this->vertices = IndexedVertexBuffer(device, lengthof(VERTICES), VERTICES, lengthof(INDICES), INDICES);
		this->shader = Shader(device, shaderPath.c_str(), SimpleVertex::LAYOUT, lengthof(SimpleVertex::LAYOUT));
	}
};

enum ConstantBufferSlot
{
	CONSTANT_BUFFER_CAMERA,
	CONSTANT_BUFFER_RENDERABLE,
};

class CubeVisualizer : public Dance::Three::ThreeVisualizer, public Dance::Audio::AudioVisualizer
{
public:
	CubeVisualizer(const Visualizer::Dependencies& dependencies, const std::filesystem::path& path) 
		: path(path)
		, size{}
		, ThreeVisualizer(dependencies)
		, AudioVisualizer(dependencies)
	{
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->cube = Cube(this->d3dDevice, (this->path.parent_path() / "Shader" / "Mesh.hlsl").wstring());
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

		this->camera.Activate(CONSTANT_BUFFER_CAMERA);
		this->cube.Render(CONSTANT_BUFFER_RENDERABLE);
		this->dxgiSwapChain->Present(1, 0);
	}

	virtual void Update(double delta)
	{
		AudioVisualizer::Update(delta);
		const size_t normalize = this->analyzer.Spectrum().size();

		FLOAT level = 0.0f;
		for (size_t i = 100; i < 1000; ++i)
		{
			level += this->analyzer.Spectrum()[i].magnitude(normalize);
		}

		this->theta += delta;
		this->cube.Transform() = Matrix4F::Scale(200.0f * level + 100.0f)
			* Matrix4F::YRotation(this->theta)
			* Matrix4F::XRotation(0.45f * this->theta)
			* Matrix4F::ZRotation(0.85f * this->theta);
	}

protected:
	std::filesystem::path path;
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
		this->camera.Projection = Matrix4F::YRotation(-PiOver2)
			* Matrix4F::ZRotation(-PiOver2)
			* Matrix4F::Perspective(
				ToRadians(70.0f),
				(FLOAT)(size.right - size.left),
				(FLOAT)(size.bottom - size.top),
				25.0f,
				10000.0f);
		this->camera.WorldToCamera = Matrix4F::Translation(Vector3(500.0f, 0.0f, 0.0f));
		return S_OK;
	}
};

extern "C"
{
	__declspec(dllexport) Visualizer* Factory(const Visualizer::Dependencies& dependencies, const std::filesystem::path& path)
	{
		return new CubeVisualizer(dependencies, path);
	}

	__declspec(dllexport) std::wstring Name()
	{
		return L"Cube";
	}
}
