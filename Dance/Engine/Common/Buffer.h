#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

const UINT ZERO = 0;

template<typename T>
class Buffer
{
public:
    UINT Count;

    Buffer(UINT count) : Count(count) {}

    inline UINT Size() const
    {
        return this->Count * sizeof(T);
    }

    inline UINT Stride() const
    {
        return static_cast<UINT>(sizeof(T));
    }
};


template<typename T>
class VertexBuffer : public Buffer<T>
{
public:
    VertexBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

	VertexBuffer(ID3D11Device* device, UINT count, const T* data) : Buffer<T>(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDescriptor.ByteWidth = this->Size();
        D3D11_SUBRESOURCE_DATA subresourceData = { data };
        OKE(device->CreateBuffer(&bufferDescriptor, &subresourceData, this->buffer.ReleaseAndGetAddressOf()));
    }

    inline void Set() const
    {
        UINT stride = this->Stride();
        this->deviceContext->IASetVertexBuffers(0, 1, this->buffer.GetAddressOf(), &stride, &ZERO);
    }

    inline void Draw() const
    {
        this->deviceContext->Draw(this->Count, 0);
    }

private:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11Buffer> buffer;
};

template<typename T, DXGI_FORMAT TF>
class IndexBuffer : public Buffer<T>
{
public:
    IndexBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

    IndexBuffer(ID3D11Device* device, UINT count, const T* data) : Buffer<T>(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDescriptor.ByteWidth = this->Size();
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
};

template<typename I, DXGI_FORMAT IF, typename V>
class IndexedVertexBuffer
{
public:
    IndexBuffer<I, IF> Indices;
    VertexBuffer<V> Vertices;

    IndexedVertexBuffer() : Indices(), Vertices() {}

    IndexedVertexBuffer
    (
        ID3D11Device* device,
        UINT indexCount,
        I* indexData,
        UINT vertexCount,
        V* vertexData
    )
        : Indices(device, indexCount, indexData)
        , Vertices(device, vertexCount, vertexData)
    {}

    inline void Set() const
    {
        this->Indices.Set();
        this->Vertices.Set();
    }

    inline void Draw() const
    {
        this->Indices.Draw();
    }
};

template<typename T>
class ConstantBuffer : public Buffer<T>
{
public:
    ConstantBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

    ConstantBuffer(ID3D11Device* device, UINT count, const T* data) : Buffer<T>(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.ByteWidth = this->Size();
        D3D11_SUBRESOURCE_DATA subresourceData = { data };
        OKE(device->CreateBuffer(&bufferDescriptor, &subresourceData, this->buffer.ReleaseAndGetAddressOf()));
    }

    inline void Set(UINT index, UINT count) const
    {
        this->deviceContext->VSSetConstantBuffers(index, count, this->buffer.GetAddressOf());
        this->deviceContext->PSSetConstantBuffers(index, count, this->buffer.GetAddressOf());
    }

private:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11Buffer> buffer;
};

template<typename T>
class MutableConstantBuffer : public ConstantBuffer<T>
{
    MutableConstantBuffer() : ConstantBuffer<T>() {}

    MutableConstantBuffer(ID3D11Device* device, UINT count, const T* data) : Buffer<T>(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.CPUAccessFlags = D3D11_USAGE_DYNAMIC;
        bufferDescriptor.ByteWidth = count * sizeof(T);
        D3D11_SUBRESOURCE_DATA subresourceData = { data };
        OKE(device->CreateBuffer(&bufferDescriptor, &subresourceData, this->buffer.ReleaseAndGetAddressOf()));
    }

    inline void Write(const T* data) const
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        OKE(this->deviceContext->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
        memcpy(mappedSubresource.pData, data, this->Size());
        this->deviceContext->Unmap(this->buffer.Get(), 0);
    }
};
