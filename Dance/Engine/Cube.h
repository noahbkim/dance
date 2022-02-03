#pragma once

#include "Engine/D3D/Renderable.h"
#include "Common/Buffer.h"
#include "Common/Primitive.h"
#include "Mathematics.h"

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

const static D3D11_INPUT_ELEMENT_DESC LAYOUT[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, offsetof(Vertex, UV),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

class Cube : public Renderable
{
public:
	Cube() : Renderable() {}

	Cube(ComPtr<ID3D11Device> device) : Renderable(device) 
	{
		this->vertices = IndexedVertexBuffer(device, lengthof(VERTICES), VERTICES, lengthof(INDICES), INDICES);
		this->shader = Shader(device, L"Shader/Mesh.hlsl", LAYOUT, lengthof(LAYOUT));
	}
};