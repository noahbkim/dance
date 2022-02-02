#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

const UINT ZERO = 0;

template<typename T>
class VertexBuffer
{
public:
    VertexBuffer() : deviceContext(nullptr), buffer(nullptr), count(0) {}

	VertexBuffer(ID3D11Device* device, UINT count, T* data) : count(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDescriptor.ByteWidth = count * sizeof(T);
        D3D11_SUBRESOURCE_DATA subresourceData = { data };
        OKE(device->CreateBuffer(&bufferDescriptor, &subresourceData, this->buffer.ReleaseAndGetAddressOf()));
    }

    inline void Set() const
    {
        UINT stride = sizeof(T);
        this->deviceContext->IASetVertexBuffers(0, 1, this->buffer.GetAddressOf(), &stride, &ZERO);
    }

    inline void Draw() const
    {
        this->deviceContext->Draw(this->count, 0);
    }

private:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11Buffer> buffer;
    UINT count;
};


template<typename T, DXGI_FORMAT TF>
class IndexBuffer
{
public:
    IndexBuffer() : deviceContext(nullptr), buffer(nullptr), count(0) {}

    IndexBuffer(ID3D11Device* device, UINT count, T* data) : count(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDescriptor.ByteWidth = count * sizeof(T);
        D3D11_SUBRESOURCE_DATA subresourceData = { data };
        OKE(device->CreateBuffer(&bufferDescriptor, &subresourceData, this->buffer.ReleaseAndGetAddressOf()));
    }

    inline void Set() const
    {
        this->deviceContext->IASetIndexBuffer(this->buffer.Get(), TF, 0);
    }

    inline void Draw() const
    {
        this->deviceContext->DrawIndexed(this->Count, 0, 0);
    }

private:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11Buffer> buffer;
    UINT count;
};

template<typename I, DXGI_FORMAT IF, typename V>
class IndexedVertexBuffer
{
public:
    IndexedVertexBuffer() : indices(), vertices() {}

    IndexedVertexBuffer
    (
        ID3D11Device* device,
        UINT indexCount,
        I* indexData,
        UINT vertexCount,
        V* vertexData
    )
        : indices(device, indexCount, indexData)
        , vertices(device, vertexCount, vertexData)
    {}

    inline void Set() const
    {
        this->vertices.Set();
        this->indices.Set();
    }

    inline void Draw() const
    {
        this->indices.Draw();
    }

private:
    IndexBuffer<I, IF> indices;
    VertexBuffer<V> vertices;
};
