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

	VertexBuffer(ComPtr<ID3D11Device> device, UINT count, const T* data) : Buffer<T>(count)
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

    IndexBuffer(ComPtr<ID3D11Device> device, UINT count, const T* data) : Buffer<T>(count)
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

template<typename V, typename I = uint16_t, DXGI_FORMAT IF = DXGI_FORMAT_R16_UINT>
class IndexedVertexBuffer
{
public:
    VertexBuffer<V> Vertices;
    IndexBuffer<I, IF> Indices;

    IndexedVertexBuffer() : Vertices(), Indices() {}

    IndexedVertexBuffer
    (
        ComPtr<ID3D11Device> device,
        UINT vertexCount,
        const V* vertexData,
        UINT indexCount,
        const I* indexData
    )
        : Vertices(device, vertexCount, vertexData)
        , Indices(device, indexCount, indexData)
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

    ConstantBuffer(UINT count) : Buffer<T>(count) {}

    ConstantBuffer(ComPtr<ID3D11Device> device, UINT count, const T* data) : Buffer<T>(count)
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

protected:
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<ID3D11Buffer> buffer;
};

template<typename T>
class MutableConstantBuffer : public ConstantBuffer<T>
{
public:
    MutableConstantBuffer() : ConstantBuffer<T>() {}

    MutableConstantBuffer(ComPtr<ID3D11Device> device, UINT count, const T* data) : ConstantBuffer<T>(count)
    {
        device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
        D3D11_BUFFER_DESC bufferDescriptor{};
        bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
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

template<typename T>
class ManagedMutableConstantBuffer : public MutableConstantBuffer<T>
{
public:
    T Data;

    ManagedMutableConstantBuffer() : MutableConstantBuffer<T>() {}

    ManagedMutableConstantBuffer(ComPtr<ID3D11Device> device)
        : MutableConstantBuffer<T>(device, 1, &this->Data)
    {}

    ManagedMutableConstantBuffer(ComPtr<ID3D11Device> device, T data)
        : Data(data)
        , MutableConstantBuffer<T>(device, 1, &this->Data)
    {}

    inline void Write() const
    {
        MutableConstantBuffer<T>::Write(&this->Data);
    }
};
