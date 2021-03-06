#pragma once

#include "Three.h"
#include "Buffer.h"
#include "Primitive.h"
#include "Shader.h"
#include "Mathematics.h"

namespace Dance::Three::Graphics
{
	using Vertex = SimpleVertex::Structure;
	using Matrix4F = Dance::Three::Math::Matrix4F;

	/// A rudimentary container for an object that is rendered via shader.
	class Renderable
	{
	public:
		/// Container class for our constants. This probably needs to be more sophisticated.
		struct Constants
		{
			Matrix4F Transform;
		};

		/// Empty initialize a new renderable. Makes no allocations.
		Renderable()
			: deviceContext(nullptr)
			, vertices()
			, constants()
			, shader()
		{}

		/// Create a new renderable using a device to allocate everything.
		/// 
		/// @param device expects a D3D11 device to initialize the mutable constant buffer with.
		/// @throws ComError if the constant buffer, index buffer, or shader fails to allocate.
		Renderable(ComPtr<ID3D11Device> device)
			: deviceContext(nullptr)
			, vertices()
			, constants(device)
			, shader()
		{
			device->GetImmediateContext(this->deviceContext.ReleaseAndGetAddressOf());
		}

		/// Set the constant buffer, write our data, apply the shader, set and draw the vertices.
		///
		/// @param index is the constant buffer index we will write to.
		virtual void Render(UINT index) {
			this->constants.Set(index);
			this->constants.Write();
			this->shader.Apply();
			this->vertices.Set();
			this->vertices.Draw();
		}

		/// Get mutable access to the renderable's transform.
		/// 
		/// @returns a const reference to the underlying Matrix4F transform in our managed constant buffer.
		Matrix4F& Transform()
		{
			return this->constants.Data.Transform;
		}

	protected:
		/// Handle to the device context, which we use for allocating a couple things.
		ComPtr<ID3D11DeviceContext> deviceContext;

		/// Container for the renderable's vertices.
		IndexedVertexBuffer<Vertex> vertices;

		/// Constant buffer and data for use in the vertex and pixel shaders.
		ManagedMutableConstantBuffer<Constants> constants;

		/// The shader with which to draw our vertices.
		Shader shader;
	};
}
