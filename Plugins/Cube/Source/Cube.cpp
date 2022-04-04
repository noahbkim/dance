#include "Cube.h"
#include "Path.h"

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

Cube::Cube() : Renderable() 
{

}

Cube::Cube(ComPtr<ID3D11Device> device, const std::wstring& shaderPath) : Renderable(device)
{
	this->vertices = IndexedVertexBuffer(device, lengthof(VERTICES), VERTICES, lengthof(INDICES), INDICES);
	this->shader = Shader(device, shaderPath.c_str(), SimpleVertex::LAYOUT, lengthof(SimpleVertex::LAYOUT));
}

CubeVisualizer::CubeVisualizer(const Visualizer::Dependencies& dependencies)
	: size{}
	, ThreeVisualizer(dependencies)
	, AudioVisualizer(dependencies)
{
	this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->cube = Cube(this->d3dDevice, (DllPath.parent_path() / "Shader" / "Mesh.hlsl").wstring());
	this->camera = Camera(this->d3dDevice, Matrix4F(), Matrix4F());
	this->theta = 0.0f;

	RECT size;
	::GetClientRect(dependencies.Window, &size);
	this->Resize(size);
}

HRESULT CubeVisualizer::Unsize()
{
	OK(ThreeVisualizer::Unsize());
	return S_OK;
}

HRESULT CubeVisualizer::Resize(const RECT& size)
{
	this->size = size;
	OK(ThreeVisualizer::Resize(size));
	OK(this->SetViewport(size));
	OK(this->SetProjection(size));
	return S_OK;
}

void CubeVisualizer::Render()
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

void CubeVisualizer::Update(double delta)
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

HRESULT CubeVisualizer::SetViewport(const RECT& size)
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

HRESULT CubeVisualizer::SetProjection(const RECT& size)
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

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DllPath = GetModulePath(instance);
        Dance::API::Register(L"Cube", Visualizer::New<CubeVisualizer>, Visualizer::Delete);
    }
    return TRUE;
}
