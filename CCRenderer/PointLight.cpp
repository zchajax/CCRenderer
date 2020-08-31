#include "PointLight.h"
#include "RenderContext.h"

PointLight* PointLight::Create(const XMFLOAT4& position, const XMFLOAT4& color, float intensity)
{
	PointLight* light = new PointLight();
	light->Init(position, color, intensity);
	return light;
}

PointLight::PointLight()
{
	m_Type = Light::POINT;
}

PointLight::~PointLight()
{

}

void PointLight::Init(const XMFLOAT4& position, const XMFLOAT4& color, float intensity)
{
	m_Color = color;
	m_Position = position;
	m_Intensity = intensity;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantPointLightBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
}

void PointLight::Apply()
{
	// update light data
	ConstantPointLightBuffer cb;
	cb.vLightPos = m_Position;
	cb.vLightColor = m_Color;
	cb.fIntensity = m_Intensity;
	RENDER_CONTEXT::GetImmediateContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// bind
	RENDER_CONTEXT::GetImmediateContext()->PSSetConstantBuffers(13, 1, &m_pConstantBuffer);
}