#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:

	Camera();
	
	~Camera();

	void Init();

	void Update(float delta);

	void Render();

	void SetViewParams(const XMFLOAT4& Eye, const XMFLOAT4& LookAt);

	void SetViewParams(const XMFLOAT4& Eye, const XMFLOAT4& LookAt, const XMFLOAT4& UP);

	void SetProjParams(FLOAT FLFovAngleY, FLOAT AspectRatio, FLOAT NearZ, FLOAT FarZ);

	void SetEye(const XMFLOAT4& Eye);

	void SetLookAt(const XMFLOAT4& LookAt);

    void SetFovAngleY(FLOAT FLFovAngleY);

    void SetAspectRatio(FLOAT AspectRatio);

	void SetNearZ(FLOAT NearZ);

	void SetFarZ(FLOAT FarZ);

	const XMFLOAT4& GetEye() const
	{
		return m_vEey;
	}

	const XMFLOAT4& GetLookAt() const
	{
		return m_vLookAt;
	}

	const XMFLOAT4& GetUp() const
	{
		return m_UpVector;
	}

	FLOAT GetNearZ() const
	{
		return m_NearZ;
	}

	FLOAT GetFarZ() const
	{
		return m_FarZ;
	}

	FLOAT GetFLFovAngleY() const
	{
		return m_FLFovAngleY;
	}

	FLOAT GetAspectRatio() const
	{
		return m_AspectRatio;
	}

	const XMFLOAT4& GetLookVector() const
	{
		return m_LookVector;
	}

	const XMFLOAT4& GetRightVector() const
	{
		return m_RightVector;
	}

	const XMFLOAT4X4& GetViewMatrix() const
	{
		return m_mViewMatrix;
	}

	const XMFLOAT4X4& GetProjMatrix() const
	{
		return m_mProjMatrix;
	}

private:

	void CalculateViewMatrix();

	void CalculateProjMatrix();

	ID3D11Buffer*	m_pConstantBuffer;
	XMFLOAT4	m_vEey;
	XMFLOAT4	m_vLookAt;
	XMFLOAT4	m_UpVector;
	XMFLOAT4	m_LookVector;
	XMFLOAT4	m_RightVector;
	FLOAT		m_FLFovAngleY;
	FLOAT		m_AspectRatio;
	FLOAT		m_NearZ;
	FLOAT		m_FarZ;
	XMFLOAT4X4	m_mViewMatrix;
	XMFLOAT4X4	m_mProjMatrix;
	BOOL		m_IsDirty;
};
