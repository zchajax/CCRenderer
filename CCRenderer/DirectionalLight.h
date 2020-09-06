#pragma once
#include "Light.h"
#include <vector>

class Node;
class CascadedShadowMap;
class DirectionalLight : public Light
{
public:

	static DirectionalLight* Create(
		const XMFLOAT4& color, 
		const XMFLOAT4& position, 
		const XMFLOAT4& direction,
		const XMFLOAT4 ambient,
		float intensity, 
		float shadowRange, 
		BOOL enableShadow = false);

	DirectionalLight();

	virtual ~DirectionalLight();

	void Init(
		const XMFLOAT4& color, 
		const XMFLOAT4& position,
		const XMFLOAT4& direction, 
		const XMFLOAT4 ambient,
		float intensity, 
		float shadowRange,
		BOOL enableShadow);

	virtual void Update(float delta) override;

	virtual void Apply() override;

	void RenderToShadowMap(const std::vector<Node*>& nodes);

	const XMFLOAT4X4& GetLightView() const
	{
		return m_LightView;
	}

	const XMFLOAT4X4& GetLightProj() const
	{
		return m_LightProj;
	}

	BOOL IsEnableShadow() const
	{
		return m_EnableShadow;
	}

private:

	struct ConstantDirectionalLightBuffer
	{
		XMFLOAT4 vAmbientColor;
		XMFLOAT4 vLightDir;
		XMFLOAT4 vLightColor;
		XMMATRIX mLightView;
		XMMATRIX mLightProj;
		FLOAT	 fIntensity;
		FLOAT	 padding1;
		FLOAT	 padding2;
		FLOAT	 padding3;
	};

	void CalculateViewMatrix();

	void CalculateProjMatrix();


	CascadedShadowMap* m_pCascadedShadowMap;

	BOOL				m_EnableShadow;

	FLOAT				m_ShadowRange;
	XMFLOAT4			m_LightDir;
	XMFLOAT4X4			m_LightView;
	XMFLOAT4X4			m_LightProj;

	XMFLOAT4			m_AmbientColor;
};