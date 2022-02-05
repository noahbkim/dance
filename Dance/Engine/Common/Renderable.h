#pragma once

#include "Framework.h"
#include "Common/Pointer.h"
#include "Engine/Common/Buffer.h"
#include "Engine/Common/Primitive.h"
#include "Engine/Common/Shader.h"
#include "Mathematics.h"

enum ConstantBufferSlot
{
	CONSTANT_BUFFER_CAMERA,
	CONSTANT_BUFFER_RENDERABLE,
};

class Renderable
{
public:
	struct Constants
	{
		Matrix4F Transform;
	};

	Renderable() 
		: deviceContext(nullptr)
		, vertices()
		, constants()
		, shader()
	{}

	Renderable(ComPtr<ID3D11Device> device) 
		: deviceContext(nullptr)
		, vertices()
		, constants(device)
		, shader() 
	{
		device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
	}

	virtual void Render() {
		this->constants.Set(CONSTANT_BUFFER_RENDERABLE, 1);
		this->constants.Write();
		this->shader.Apply();
		this->vertices.Set();
		this->vertices.Draw();
	}

	Matrix4F& Transform()
	{
		return this->constants.Data.Transform;
	}

protected:
	ComPtr<ID3D11DeviceContext> deviceContext;
	IndexedVertexBuffer<Vertex> vertices;
	ManagedMutableConstantBuffer<Constants> constants;
	Shader shader;
};
