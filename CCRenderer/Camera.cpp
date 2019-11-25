#include "Camera.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "commonh.h"
#include <assert.h>

Camera::Camera()
{
	m_pConstantBuffer = NULL;

	m_IsDirty = true;
}

Camera::~Camera()
{
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
}

void Camera::Init()
{
	auto Eye = XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f);
	auto At = XMFLOAT4(0.0f, 0.0f, -4.0f, 1.0f);
	auto Up = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	SetViewParams(Eye, At, Up);
	SetProjParams(XM_PIDIV2, 1.0f, 0.01f, 100.0f);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantCameraBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
}

void Camera::Update(float delta)
{
	GameApp* app = GameApp::getInstance();

	if (m_IsDirty)
	{
		m_IsDirty = false;
	}
}

void Camera::Render()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	// Update constant buffer
	ConstantCameraBuffer cb;
	cb.mView = XMMatrixTranspose(XMLoadFloat4x4(&m_mViewMatrix));
	cb.mProjection = XMMatrixTranspose(XMLoadFloat4x4(&m_mProjMatrix));
	cb.vEye = m_vEey;
	immediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set constant  buffer
	immediateContext->VSSetConstantBuffers(11, 1, &m_pConstantBuffer);
	immediateContext->PSSetConstantBuffers(11, 1, &m_pConstantBuffer);
}

void Camera::SetViewParams(const XMFLOAT4& Eye, const XMFLOAT4& LookAt)
{
	m_vEey = Eye;
	m_vLookAt = LookAt;

	CalculateViewMatrix();
}

void Camera::SetViewParams(const XMFLOAT4& Eye, const XMFLOAT4& LookAt, const XMFLOAT4& Up)
{
	m_UpVector = Up;

	SetViewParams(Eye, LookAt);
}

void Camera::SetProjParams(FLOAT FLFovAngleY,
	FLOAT AspectRatio,
	FLOAT NearZ,
	FLOAT FarZ)
{
	m_FLFovAngleY = FLFovAngleY;
	m_AspectRatio = AspectRatio;
	m_NearZ = NearZ;
	m_FarZ = FarZ;

	CalculateProjMatrix();
}

void Camera::SetEye(const XMFLOAT4& Eye)
{
	m_vEey = Eye;

	CalculateViewMatrix();
}

void Camera::SetLookAt(const XMFLOAT4& lookAt)
{
	m_vLookAt = lookAt;

	CalculateViewMatrix();
}

void Camera::SetFovAngleY(FLOAT FLFovAngleY)
{
	m_FLFovAngleY = FLFovAngleY;

	CalculateProjMatrix();
}

void Camera::SetAspectRatio(FLOAT AspectRatio)
{
	m_AspectRatio = AspectRatio;

	CalculateProjMatrix();
}

void Camera::SetNearZ(FLOAT NearZ)
{
	m_NearZ = NearZ;

	CalculateProjMatrix();
}

void Camera::SetFarZ(FLOAT FarZ)
{
	m_FarZ = FarZ;

	CalculateProjMatrix();
}

void Camera::CalculateViewMatrix()
{
	m_IsDirty = true;

	auto lookAt = XMLoadFloat4(&m_vLookAt);
	auto eye = XMLoadFloat4(&m_vEey);

	auto lookVector = XMVector3Normalize(lookAt - eye);
	auto rightVector = XMVector3Normalize(XMVector3Cross(XMLoadFloat4(&m_UpVector), lookVector));
	auto upVector = XMVector3Normalize(XMVector3Cross(lookVector, rightVector));

	XMStoreFloat4(&m_LookVector, lookVector);
	XMStoreFloat4(&m_RightVector, rightVector);
	XMStoreFloat4(&m_UpVector, upVector);

	XMStoreFloat4x4(&m_mViewMatrix, XMMatrixLookAtLH(eye, lookAt, upVector));
}

void Camera::CalculateProjMatrix()
{
	m_IsDirty = true;

	XMStoreFloat4x4(&m_mProjMatrix, XMMatrixPerspectiveFovLH(m_FLFovAngleY, m_AspectRatio, m_NearZ, m_FarZ));
}