#pragma once
#include "Light.h"

class DirectionalLight : public Light
{
public:

	static DirectionalLight* Create(const XMFLOAT4& color, const XMFLOAT4& position, const XMFLOAT4& direction, const XMFLOAT4 ambient, float intensity, float shadowRange);

	DirectionalLight();

	virtual ~DirectionalLight();

	void Init(const XMFLOAT4& color, const XMFLOAT4& position, const XMFLOAT4& direction, const XMFLOAT4 ambient, float intensity, float shadowRange);

	virtual void Apply() override;

	const XMFLOAT4X4& GetLightView() const
	{
		return m_LightView;
	}

	const XMFLOAT4X4& GetLightProj() const
	{
		return m_LightProj;
	}

private:

	struct ConstantDirectionalLightBuffer
	{
		XMFLOAT4 vAmbientColor;
		XMFLOAT4 vLightDir;
		XMFLOAT4 vLightColor;
		XMMATRIX mLightView;
		XMMATRIX mLightProj;
	};

	void CalculateViewMatrix();

	void CalculateProjMatrix();

	FLOAT		m_ShadowRange;
	XMFLOAT4	m_LightDir;
	XMFLOAT4X4	m_LightView;
	XMFLOAT4X4	m_LightProj;

	XMFLOAT4	m_AmbientColor;
};