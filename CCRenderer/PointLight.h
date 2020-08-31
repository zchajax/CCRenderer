#pragma once
#include "Light.h"

class PointLight : public Light
{
public:

	static PointLight* Create(const XMFLOAT4& position, const XMFLOAT4& color, float intensity);

	PointLight();

	virtual ~PointLight();

	void Init(const XMFLOAT4& position, const XMFLOAT4& color, float intensity);

	virtual void Apply() override;

private:

	struct ConstantPointLightBuffer
	{
		XMFLOAT4 vLightPos;
		XMFLOAT4 vLightColor;
		FLOAT	 fIntensity;
		FLOAT	 padding1;
		FLOAT	 padding2;
		FLOAT	 padding3;
	};
};