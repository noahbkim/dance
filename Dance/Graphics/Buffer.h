#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

const UINT ZERO = 0;

/// Generalized buffer manager. Does not actually implement any memory management details, just provides convenience
/// calculations to do with absolute size, item count, and stride length.
/// 
/// @typeparam T the type/format of the buffer item or items.
template<typename T>
class Buffer
{
public:
    /// Initialize a new buffer with its size.
    /// 
    /// @param count indicates the size in terms of the number of T's it contains.
    Buffer(UINT count) : count(count) {}

    /// Compute the size in bytes of the buffer.
    /// 
    /// @returns the product of the buffer's count and the sizeof(T).
    inline constexpr UINT Size() const
    {
        return this->count * sizeof(T);
    }

    /// Get the count the buffer was initialized to.
    /// 
    /// @returns the buffer size in terms of the number of T's it contains.
    inline constexpr UINT Count() const
    {
        return this->count;
    }

    /// Get the byte width of a single item in the buffer.
    /// 
    /// @returns the sizeof(T) as a UINT.
    inline constexpr UINT Stride() const
    {
        return static_cast<UINT>(sizeof(T));
    }

protected:
    /// Count of items in buffer. We use UINT because that's what DirectX uses expects in D3D11_BUFFER_DESC.
    UINT count;
};

/// An immutable buffer bound as a D3D11_BIND_VERTEX_BUFFER.
/// 
/// @typeparam T the underlying vertex type.
template<typename T>
class VertexBuffer : public Buffer<T>
{
public:
    /// Initialize an empty vertex buffer. Does no allocation.
    VertexBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

	/// Initialize a new vertex buffer using the D3D11 device.
    /// 
	/// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
	/// @param count is the number of vertices the buffer should contain.
	/// @param data a pointer to the data to write to the buffer, required as it is immutable.
    /// @throws a ComError if ID3D11Device::CreateBuffer returns an invalid result.
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

    /// Set the device context's active vertex buffer.
    inline void Set() const
    {
        UINT stride = this->Stride();
        this->deviceContext->IASetVertexBuffers(0, 1, this->buffer.GetAddressOf(), &stride, &ZERO);
    }

    /// Draw the vertex buffer to the device context.
    inline void Draw() const
    {
        this->deviceContext->Draw(this->count, 0);
    }

private:
    /// We have to keep track of the device context in order to set and draw the buffer.
    ComPtr<ID3D11DeviceContext> deviceContext;

    /// Underlying pointer to the buffer.
    ComPtr<ID3D11Buffer> buffer;
};

/// An immutable buffer bound as a D3D11_BIND_INDEX_BUFFER.
/// 
/// @typeparam T is the index type.
/// @typeparam TF is the DXGI_FORMAT of T, which simply indicates the integer type.
template<typename T, DXGI_FORMAT TF>
class IndexBuffer : public Buffer<T>
{
public:
    /// Initialize an empty index buffer. Makes no allocations.
    IndexBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

    /// Allocate a new index buffer using a D3D11 device.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param count is the number of indices the buffer should contain.
    /// @param data a pointer to the index data to write to the buffer, required as it is immutable.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
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

    /// Set the device context's active vertex buffer.
    inline void Set(UINT offset = 0) const
    {
        this->deviceContext->IASetIndexBuffer(this->buffer.Get(), TF, offset);
    }

    /// Draw the current vertex buffer using the indices contained in our buffer.
    inline void Draw(UINT startIndex = 0, UINT baseVertexIndex = 0) const
    {
        this->deviceContext->DrawIndexed(this->count, startIndex, baseVertexIndex);
    }

private:
    /// We have to keep track of the device context in order to set and draw the buffer.
    ComPtr<ID3D11DeviceContext> deviceContext;

    /// Underlying pointer to the buffer.
    ComPtr<ID3D11Buffer> buffer;
};

/// An immutable, indexed vertex buffer. Surprisingly, consists of a vertex buffer and an index buffer.
/// 
/// @typeparam V is the vertex type, as passed to T in VertexBuffer.
/// @typeparam I is the index type, as passed to T in IndexBuffer.
/// @typeparam IF is the index format, as passed to TF in IndexBuffer.
template<typename V, typename I = uint16_t, DXGI_FORMAT IF = DXGI_FORMAT_R16_UINT>
class IndexedVertexBuffer
{
public:
    /// Contains the condensed set of vertices.
    VertexBuffer<V> Vertices;

    /// Contains indices to draw from while rendering the model.
    IndexBuffer<I, IF> Indices;

    /// Initialize a new indexed vertex buffer. Makes no allocations.
    IndexedVertexBuffer() : Vertices(), Indices() {}

    /// Initialize a new indexed vertex buffer.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param vertexCount is the number of vertices the buffer should contain.
    /// @param vertexData a pointer to the vertex data to write to the vertex buffer, required as it is immutable.
    /// @param indexCount is the number of indices the buffer should contain.
    /// @param indexData a pointer to the index data to write to the index buffer, required as it is immutable.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
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

    /// Set the index and vertex buffer for the device context.
    inline void Set() const
    {
        this->Indices.Set();
        this->Vertices.Set();
    }

    /// Draw the vertices via the index buffer.
    inline void Draw() const
    {
        this->Indices.Draw();
    }
};

/// A managed, immutable buffer bound as a D3D11_BIND_CONSTANT_BUFFER. The constant in its name does not refer to its
/// immutability; it's just the canonical term for non-vertex/index data.
/// 
/// @typeparam T is the type/format of the buffer's contents.
template<typename T>
class ConstantBuffer : public Buffer<T>
{
public:
    /// Initialize an empty constant buffer. Makes no allocations.
    ConstantBuffer() : Buffer<T>(0), deviceContext(nullptr), buffer(nullptr) {}

    /// Allocate a new constant buffer using a D3D11 device.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param count is the number of items the buffer should contain.
    /// @param data a pointer to the data to write to the buffer, required as it is immutable.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
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

    /// Set this buffer to one of constant buffer indices for both the vertex and pixel shaders.
    /// 
    /// @param index is the index we set the buffer to.
    /// @param count is the number of items we want to write to the buffer.
    inline void Set(UINT index) const
    {
        this->deviceContext->VSSetConstantBuffers(index, this->count, this->buffer.GetAddressOf());
        this->deviceContext->PSSetConstantBuffers(index, this->count, this->buffer.GetAddressOf());
    }

protected:
    /// We have to keep track of the device context in order to set and draw the buffer.
    ComPtr<ID3D11DeviceContext> deviceContext;

    /// Underlying pointer to the buffer.
    ComPtr<ID3D11Buffer> buffer;

    /// A convenience constructor used by the mutable version of this buffer.
    /// 
    /// @param count represents the number of T's this buffer should contain.
    ConstantBuffer(UINT count) : Buffer<T>(count), deviceContext(nullptr), buffer(nullptr) {}
};

/// A mutable version of the constant buffer. Provides overwrite functionality.
/// 
/// @typeparam T is the type/format of the buffer's contents.
template<typename T>
class MutableConstantBuffer : public ConstantBuffer<T>
{
public:
    /// Initialize an empty constant buffer. Makes no allocations.
    MutableConstantBuffer() : ConstantBuffer<T>() {}

    /// Allocate a new constant buffer using a D3D11 device. Sets usage to D3D11_USAGE_DYNAMIC.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param count is the number of items the buffer should contain.
    /// @param data a pointer to the data to write to the buffer, required as it is immutable.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
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

    /// Write new data to the buffer. First maps the buffer to a memory address, then copies over the contents of the
    /// data pointer, and finally unmaps the buffer.
    /// 
    /// @param data the new data to write to the buffer.
    /// @throws ComError if ID3D11DeviceContext::Map returns an invalid result.
    inline void Write(const T* data) const
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        OKE(this->deviceContext->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
        memcpy(mappedSubresource.pData, data, this->Size());
        this->deviceContext->Unmap(this->buffer.Get(), 0);
    }
};

/// A constant buffer for a single T with a local instance provided within the class itself. Count is implicitly one.
/// 
/// @typeparam T is the type/format of the buffer's contents.
template<typename T>
class ManagedMutableConstantBuffer : public MutableConstantBuffer<T>
{
public:
    /// The data source we're writing from.
    T Data;

    /// Initialize an empty constant buffer. Makes no allocations.
    ManagedMutableConstantBuffer() : MutableConstantBuffer<T>() {}

    /// Allocatae the constant buffer using the D3D11 device and write the default-constructed data into it.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
    /// @see MutableConstantBuffer::MutableConstantBuffer
    ManagedMutableConstantBuffer(ComPtr<ID3D11Device> device)
        : MutableConstantBuffer<T>(device, 1, &this->Data)
    {}

    /// Allocatae the constant buffer using the D3D11 device and copy-construct data before writing it.
    /// 
    /// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
    /// @param data a pointer to the data to write to the buffer, required as it is immutable.
    /// @throws ComError if ID3D11Device::CreateBuffer returns an invalid result.
    /// @see MutableConstantBuffer::MutableConstantBuffer
    ManagedMutableConstantBuffer(ComPtr<ID3D11Device> device, T data)
        : Data(data)
        , MutableConstantBuffer<T>(device, 1, &this->Data)
    {}

    /// Overwrite MutableConstantBuffer::Write because we're always gonna write our this->Data.
    inline void Write() const
    {
        MutableConstantBuffer<T>::Write(&this->Data);
    }
};
