#pragma once

#include "Framework.h"
#include "Mathematics.h"
#include "Engine/Common/Buffer.h"
#include "Engine/Common/Renderable.h"

class Camera
{
public:
	Matrix4F WorldToCamera;
	Matrix4F Projection;

	struct Constants
	{
		Matrix4F ViewProjection;
		Vector3F Position;
		float _pad;
	};

	Camera() : constants() {}

	Camera(ComPtr<ID3D11Device> device, Matrix4F worldToCamera, Matrix4F projection)
		: WorldToCamera(worldToCamera)
		, Projection(projection)
		, constants(device)
	{}

	virtual ~Camera() {}

	void Activate()
	{
		this->constants.Data.ViewProjection = this->WorldToCamera * this->Projection;
		this->constants.Data.Position = this->WorldToCamera.Inverted().Translation();
		this->constants.Set(CONSTANT_BUFFER_CAMERA, 1);
		this->constants.Write();
	}

protected:
	ManagedMutableConstantBuffer<Constants> constants;
};
