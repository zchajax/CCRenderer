#include "DirectionalLight.h"
#include "RenderContext.h"
#include "CascadedShadowMap.h"
#include "Node.h"
#include "GameApp.h"

DirectionalLight* DirectionalLight::Create(
	const XMFLOAT4& color, 
	const XMFLOAT4& position,
	const XMFLOAT4& direction,
	const XMFLOAT4 ambient, 
	float intensity, 
	float shadowRange,
	BOOL enableShadow)
{
	DirectionalLight* light = new DirectionalLight();
	light->Init(color, position, direction, ambient, intensity, shadowRange, enableShadow);
	return light;
}

DirectionalLight::DirectionalLight() : 
	m_EnableShadow(false),
	m_pCascadedShadowMap(NULL),
	m_ShadowRange(0),
	m_LightDir(0, 0, 0, 0),
	m_AmbientColor(0, 0, 0, 0)
{
	m_Type = Light::DIRECTIONAL;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::Init(
	const XMFLOAT4& color, 
	const XMFLOAT4& position,
	const XMFLOAT4& direction,
	const XMFLOAT4 ambient,
	float intensity, 
	float shadowRange,
	BOOL enableShadow)
{
	m_Color = color;
	m_Position = position;
	m_LightDir = direction;
	m_AmbientColor = ambient;
	m_Intensity = intensity;
	m_ShadowRange = shadowRange;
	m_EnableShadow = enableShadow;

	CalculateViewMatrix();
	CalculateProjMatrix();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantDirectionalLightBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);

	//Create depth shader class
	m_pCascadedShadowMap = new CascadedShadowMap();
	m_pCascadedShadowMap->Init(1024, 1024);
}

void DirectionalLight::Update(float delta)
{
	// Update Cascaded Shadow map
	m_pCascadedShadowMap->Update(
		GameApp::getInstance()->getMainCamera(),
		XMLoadFloat4x4(&m_LightView),
		XMLoadFloat4x4(&m_LightProj));
}

void DirectionalLight::Apply()
{
	// update light data
	ConstantDirectionalLightBuffer cb;
	cb.vAmbientColor = m_AmbientColor;
	cb.vLightDir = m_LightDir;
	cb.vLightColor = m_Color;
	cb.mLightView = XMMatrixTranspose(XMLoadFloat4x4(&m_LightView));
	cb.mLightProj = XMMatrixTranspose(XMLoadFloat4x4(&m_LightProj));
	cb.fIntensity = m_Intensity;
	RENDER_CONTEXT::GetImmediateContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// bind
	RENDER_CONTEXT::GetImmediateContext()->PSSetConstantBuffers(12, 1, &m_pConstantBuffer);

	if (m_EnableShadow)
	{
		m_pCascadedShadowMap->PrepareRenderWithShadowMap(RENDER_CONTEXT::GetImmediateContext());
		RENDER_CONTEXT::SetPixelShaderResource(0, m_pCascadedShadowMap->getShadowMap());
	}
}

void DirectionalLight::RenderToShadowMap(const std::vector<Node*>& nodes)
{
	RENDER_CONTEXT::PushMarker(MARK("ShadowPass"));

	m_pCascadedShadowMap->SetViewPort(RENDER_CONTEXT::GetImmediateContext());

	for (int cascadedIndex = 0; cascadedIndex < MAX_CASCADED; cascadedIndex++)
	{
		RENDER_CONTEXT::PushMarker(MARK("Cascaded shadow"));

		m_pCascadedShadowMap->SetAndClearRenderTargetView(RENDER_CONTEXT::GetImmediateContext(), cascadedIndex);

		m_pCascadedShadowMap->RenderToDepthMap(
			RENDER_CONTEXT::GetImmediateContext(),
			cascadedIndex);

		for (auto iter = nodes.begin(); iter != nodes.end(); iter++)
		{
			if ((*iter)->GetType() == Node::NODE_TYPE_OPAQUE)
			{
				(*iter)->SetEnableBlend(false);
				(*iter)->RenderToDepthTexture();
			}
		}

		RENDER_CONTEXT::PopMarker();
	}

	RENDER_CONTEXT::PopMarker();
}

void DirectionalLight::CalculateViewMatrix()
{
	auto upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto direcionVector = XMVector3Normalize(XMLoadFloat4(&m_LightDir));
	auto rightVector = XMVector3Normalize(XMVector3Cross(upVector, direcionVector));
	upVector = XMVector3Normalize(XMVector3Cross(direcionVector, rightVector));
	XMStoreFloat4x4(&m_LightView, XMMatrixLookToLH(XMLoadFloat4(&m_Position), -direcionVector, upVector));

	XMStoreFloat4(&m_LightDir, direcionVector);
}

void DirectionalLight::CalculateProjMatrix()
{
	XMStoreFloat4x4(&m_LightProj, XMMatrixOrthographicLH(16, 16, 0.0f, m_ShadowRange));
}