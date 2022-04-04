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

static std::filesystem::path DllPath;

class Cube : public Renderable
{
public:
	Cube();
	Cube(ComPtr<ID3D11Device> device, const std::wstring& shaderPath);
};

enum ConstantBufferSlot
{
	CONSTANT_BUFFER_CAMERA,
	CONSTANT_BUFFER_RENDERABLE,
};

class CubeVisualizer : public Dance::Three::ThreeVisualizer, public Dance::Audio::AudioVisualizer
{
public:
	CubeVisualizer(const Visualizer::Dependencies& dependencies);

	virtual HRESULT Unsize();
	virtual HRESULT Resize(const RECT& size);

	virtual void Render();
	virtual void Update(double delta);

protected:
	RECT size;
	Cube cube;
	Camera camera;
	float theta;

	virtual HRESULT SetViewport(const RECT& size);
	virtual HRESULT SetProjection(const RECT& size);
};
