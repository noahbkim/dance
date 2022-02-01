#pragma once

#include "framework.h"
#include "engine.h"

class Shader
{
public:
    Shader(
        ID3D11VertexShader* vertexShader,
        ID3D11PixelShader* pixelShader,
        ID3D11InputLayout* inputLayout
    )
        : vertexShader(vertexShader)
        , pixelShader(pixelShader)
        , inputLayout(inputLayout) {}

    void Apply(ID3D11DeviceContext& deviceContext) const
    {
        deviceContext.VSSetShader(vertexShader.Get(), nullptr, 0);
        deviceContext.PSSetShader(pixelShader.Get(), nullptr, 0);
        deviceContext.IASetInputLayout(inputLayout.Get());
    }

    static ComPtr<ID3DBlob> Compile(const WCHAR* path, const char* entryPoint, const char* model)
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        // dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ComPtr<ID3DBlob> pErrorBlob = nullptr;
        ComPtr<ID3DBlob> pBlob;

        GUARD(D3DCompileFromFile(
            path,
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint,
            model,
            dwShaderFlags,
            0,
            &pBlob,
            &pErrorBlob), result)
        {
            if (pErrorBlob)
            {
                static wchar_t szBuffer[4096];
                _snwprintf_s(szBuffer, 4096, _TRUNCATE, L"%hs", (char*)pErrorBlob->GetBufferPointer());
                TRACE("error compiling shader" << szBuffer);
            }
            return nullptr;
        }

        return pBlob;
    }

    static std::unique_ptr<Shader> Load(
        ID3D11Device& device,
        const WCHAR* path,
        const D3D11_INPUT_ELEMENT_DESC* layout,
        size_t count)
    {
        ComPtr<ID3DBlob> pBlobVS = Shader::Compile(path, "VS", "vs_4_0");
        ComPtr<ID3DBlob> pBlobPS = Shader::Compile(path, "PS", "ps_4_0");

        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
        ID3D11InputLayout* inputLayout;

        GUARD(device.CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), nullptr, &vertexShader), result) 
        {
            TRACE("failed to create vertex shader: " << result);
            return nullptr;
        }

        GUARD(device.CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), nullptr, &pixelShader), result)
        {
            TRACE("failed to create vertex shader: " << result);
            return nullptr;
        }

        GUARD(device.CreateInputLayout(layout, count, pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &inputLayout), result) 
        {
            TRACE("failed to create shader input layout: " << result);
            return nullptr;
        }

        return std::make_unique<Shader>(vertexShader, pixelShader, inputLayout);
    }

private:
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
};
