#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

#ifdef _DEBUG
// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
// Setting this flag improves the shader debugging experience, but still allows 
// the shaders to be optimized and to run exactly the way they will run in 
// the release configuration of this program.
constexpr DWORD SHADER_COMPILE_FLAGS = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;  // | D3DCOMPILE_SKIP_OPTIMIZATION
#else
constexpr DWORD SHADER_COMPILE_FLAGS = D3DCOMPILE_ENABLE_STRICTNESS;
#endif


class Shader
{
public:
    Shader() : deviceContext(nullptr), vertexShader(nullptr), pixelShader(nullptr), inputLayout(nullptr) {}

    Shader
    (
        ID3D11Device* device,
        const WCHAR* path,
        const D3D11_INPUT_ELEMENT_DESC* layout,
        size_t count
    )
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());

        ComPtr<ID3DBlob> pBlobVS = Shader::Compile(path, "VS", "vs_5_0");
        ComPtr<ID3DBlob> pBlobPS = Shader::Compile(path, "PS", "ps_5_0");

        OKE(device->CreateVertexShader(
            pBlobVS->GetBufferPointer(),
            pBlobVS->GetBufferSize(),
            nullptr,
            this->vertexShader.ReleaseAndGetAddressOf()));
        OKE(device->CreatePixelShader(
            pBlobPS->GetBufferPointer(),
            pBlobPS->GetBufferSize(),
            nullptr,
            this->pixelShader.ReleaseAndGetAddressOf()));
        OKE(device->CreateInputLayout(
            layout,
            count,
            pBlobVS->GetBufferPointer(),
            pBlobVS->GetBufferSize(),
            this->inputLayout.ReleaseAndGetAddressOf()));
    }

    void Apply() const
    {
        this->deviceContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
        this->deviceContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
        this->deviceContext->IASetInputLayout(this->inputLayout.Get());
    }

    static ComPtr<ID3DBlob> Compile(const WCHAR* path, const char* entry, const char* model)
    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT result = ::D3DCompileFromFile(
            path,
            nullptr,
            nullptr,
            entry,
            model,
            0,
            0,
            shaderBlob.ReleaseAndGetAddressOf(),
            errorBlob.ReleaseAndGetAddressOf());

        if (result != S_OK)
        {
            if (errorBlob != nullptr)
            {
                std::string what(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                throw ComError(result, what);
            }
            else
            {
                throw ComError(result);
            }
        }

        return shaderBlob;
    }

private:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
};
