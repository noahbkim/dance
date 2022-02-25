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

/// Wraps rudimentary aspects of a GLSL shader. Compiles for vs_5_0 and ps_5_0.
class Shader
{
public:
    /// Empty initialize a shader. Makes no allocation but produces undefined behavior in its empty state.
    Shader() 
        : deviceContext(nullptr)
        , vertexShader(nullptr)
        , pixelShader(nullptr)
        , inputLayout(nullptr) 
    {}

    /// Construct a shader using a D3D11 device, a path to a GLSL file, and a vertex layout. Invokes Shader::Compile
    /// directly, so see that method for specifics and error handling. Entry points for the vertex and pixel shaders
    /// should be named VS and PS respectively.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param path should be a valid file path to the GLSL shader we will compile and use.
    /// @param layout must describe the vertex layout, matching both the vertex data we apply the shader to as well as
    /// the input struct declared in the actual shader file.
    /// @param layoutCount should be the number of elements in the layout array.
    /// @throws ComError if vertex shader, pixel shader, or input layout creation fails.
    /// @seealso Shader::Compile
    Shader
    (
        ComPtr<ID3D11Device> device,
        const WCHAR* path,
        const D3D11_INPUT_ELEMENT_DESC* layout,
        UINT layoutCount
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
            layoutCount,
            pBlobVS->GetBufferPointer(),
            pBlobVS->GetBufferSize(),
            this->inputLayout.ReleaseAndGetAddressOf()));
    }

    /// Apply the shader to the device context, making it so that rendered objects pass through this pipeline.
    void Apply() const
    {
        this->deviceContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
        this->deviceContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
        this->deviceContext->IASetInputLayout(this->inputLayout.Get());
    }

    /// Compile a shader file using D3D's builtin GLSL compiler.
    ///
    /// @param path should be a valid file path to the GLSL shader we will compile and use.
    /// @param entry is the entry point of the function we're compiling for.
    /// @param model is the compilation model we're targeting.
    /// @returns a ComPtr to a D3DBlob containing the compiled shader assembly.
    static ComPtr<ID3DBlob> Compile(const WCHAR* path, const char* entry, const char* model)
    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT result = ::D3DCompileFromFile(
            path,
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
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
    /// Reference to the device context for applying the shader.
    ComPtr<ID3D11DeviceContext> deviceContext;

    /// Managed vertex shader.
    ComPtr<ID3D11VertexShader> vertexShader;

    /// Managed pixel shader.
    ComPtr<ID3D11PixelShader> pixelShader;

    /// Managed input layout.
    ComPtr<ID3D11InputLayout> inputLayout;
};
