#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
using namespace DirectX;

class LightSource
{
public:
	
	LightSource();
	
	~LightSource();

	void Init();

	void Update(float delta);

	void Render();

	void SetViewParams(const XMFLOAT4& Position, const XMFLOAT4& LookAt, const XMFLOAT4& Up);

	void SetProjParams(FLOAT Width,
		FLOAT Height,
		FLOAT NearZ,
		FLOAT FarZ);

	void SetLightDir(const XMFLOAT4& dir)
	{
		m_LightDir = dir;
	}

	const XMFLOAT4& GetPosition() const
	{
		return m_Position;
	}

	const XMFLOAT4& GetLookAt() const
	{
		return m_LookAt;
	}

    const XMFLOAT4X4& GetLightView() const
    {
        return m_LightView;
    }

	const XMFLOAT4X4& GetLightProj() const
	{
		return m_LightProj;
	}

	void SetDiffuseColor(XMFLOAT4& diffuseColor)
	{
		m_LightDir = diffuseColor;
	}

	const XMFLOAT4& GetDiffuseColor() const
	{
		return m_LightColor;
	}

private:

    struct ConstantLightBuffer
    {
        XMFLOAT4 vAmbientColor;
        XMFLOAT4 vLightDir;
        XMFLOAT4 vLightColor;
        XMMATRIX mLightView;
        XMMATRIX mLightProj;
        FLOAT Shininess;
        FLOAT padding1;
        FLOAT padding2;
        FLOAT padding3;
    };

	void CalculateViewMatrix();

	void CalculateProjMatrix();

	ID3D11Buffer*	m_pConstantBuffer;

	XMFLOAT4	m_LightDir;
	XMFLOAT4	m_Position;
	XMFLOAT4	m_LookAt;
	XMFLOAT4	m_Up;
	FLOAT		m_Width;
	FLOAT		m_Height;
	FLOAT		m_NearZ;
	FLOAT		m_FarZ;
	XMFLOAT4X4	m_LightView;
	XMFLOAT4X4	m_LightProj;

	XMFLOAT4	m_AmbientColor;
	XMFLOAT4	m_LightColor;
	FLOAT		m_Shininess;
};
