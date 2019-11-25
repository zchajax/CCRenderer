#include "LightSource.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "commonh.h"

LightSource::LightSource()
{
	m_pConstantBuffer = nullptr;
}

LightSource::~LightSource()
{
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
}

void LightSource::Init()
{
	auto position = XMFLOAT4(-10.0f, 10.0f, -10.0f, 1.0f);
	auto lookAt = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	auto up = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	SetViewParams(position, lookAt, up);
	SetProjParams(20, 20, 0.01f, 20.0f);

	m_AmbientColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	XMStoreFloat4(&m_LightDir, XMVector4Normalize(XMLoadFloat4(&position) - XMLoadFloat4(&lookAt)));
	m_LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Shininess = 0.078f;

	GameApp* app = GameApp::getInstance();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantLightBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
}

void LightSource::Update(float dleta)
{
	
}

void LightSource::Render()
{
	// Update constant buffer
	ConstantLightBuffer cb;
	cb.vAmbientColor = m_AmbientColor;
	cb.vLightDir = m_LightDir;
	cb.vLightColor = m_LightColor;
	cb.Shininess = m_Shininess;
	cb.mLightView = XMMatrixTranspose(XMLoadFloat4x4(&m_LightView));
	cb.mLightProj = XMMatrixTranspose(XMLoadFloat4x4(&m_LightProj));
	RENDER_CONTEXT::GetImmediateContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set constant buffer
	RENDER_CONTEXT::GetImmediateContext()->VSSetConstantBuffers(12, 1, &m_pConstantBuffer);
	RENDER_CONTEXT::GetImmediateContext()->PSSetConstantBuffers(12, 1, &m_pConstantBuffer);
}

void LightSource::SetViewParams(const XMFLOAT4& Position, const XMFLOAT4& LookAt, const XMFLOAT4& Up)
{
	m_Position = Position;
	m_LookAt = LookAt;
	m_Up = Up;

	calculateViewMatrix();
}

void LightSource::SetProjParams(FLOAT Width,
	FLOAT Height,
	FLOAT NearZ,
	FLOAT FarZ)
{
	m_Width = Width;
	m_Height = Height;
	m_NearZ = NearZ;
	m_FarZ = FarZ;

	calculateProjMatrix();
}

void LightSource::calculateViewMatrix()
{
	auto upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto lookVector = XMVector3Normalize(XMLoadFloat4(&m_LookAt) - XMLoadFloat4(&m_Position));
	auto rightVector = XMVector3Normalize(XMVector3Cross(upVector, lookVector));
	upVector = XMVector3Normalize(XMVector3Cross(lookVector, rightVector));

	XMStoreFloat4x4(&m_LightView, XMMatrixLookAtLH(XMLoadFloat4(&m_Position), XMLoadFloat4(&m_LookAt), upVector));
}

void LightSource::calculateProjMatrix()
{
	XMStoreFloat4x4(&m_LightProj, XMMatrixOrthographicLH(16, 16, m_NearZ, m_FarZ));

	//XMStoreFloat4x4(&m_LightProj, XMMatrixPerspectiveFovLH(0.7f, m_Width / m_Height, m_NearZ, m_FarZ));
}