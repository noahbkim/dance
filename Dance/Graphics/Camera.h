#pragma once

#include "Framework.h"
#include "Mathematics/Mathematics.h"
#include "Graphics/Buffer.h"

/// A rudimentary camera implementation for DirectX 3D.
class Camera
{
public:
	/// Transforms world coordinates relative to the camera.
	Matrix4F WorldToCamera;

	/// Projection matrix for the camera.
	Matrix4F Projection;

	/// Camera constants as translated to the GPU for the shader.
	struct Constants
	{
		Matrix4F ViewProjection;
		Vector3F Position;
		float _pad;
	};

	/// Empty initialize a camera without making allocation.
	Camera() : constants() {}

	/// Initialize a camera for a D3D11 device since we need a constant buffer. Takes an initial transform and projection.
	/// 
	/// @param device expects a ComPtr to a D3D11 device, from which we extract the immediate context for later use.
	/// @param worldToCamera should be a matrix that transforms world coordinates to be relative to the camera.
	/// @param projection is the initial camera projection.
	/// @throws ComError if constant buffer allocation fails.
	Camera(ComPtr<ID3D11Device> device, Matrix4F worldToCamera, Matrix4F projection)
		: WorldToCamera(worldToCamera)
		, Projection(projection)
		, constants(device)
	{}

	/// Recompute the view projection and position. Set the constant buffer to the CONSTANT_BUFFER_CAMERA slot.
	/// 
	/// @todo move out recalculation logic since it's needlessly expensive if the camera is static. Also need to 
	/// refactor the slot system.
	void Activate()
	{
		this->constants.Data.ViewProjection = this->WorldToCamera * this->Projection;
		this->constants.Data.Position = this->WorldToCamera.Inverted().Translation();
		this->constants.Set(CONSTANT_BUFFER_CAMERA);
		this->constants.Write();
	}

protected:
	/// The mutable buffer we'll write our camera constants into.
	ManagedMutableConstantBuffer<Constants> constants;
};
