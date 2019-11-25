#include "CascadedShadowMap.h"
#include "Camera.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Texture_DX11.h"

static const XMVECTORF32 g_vFLTMAX = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
static const XMVECTORF32 g_vFLTMIN = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

CascadedShadowMap::CascadedShadowMap()
{
	m_pDepthTexture2D = NULL;
	m_pShadowMap = NULL;
	m_pDepthTarget = NULL;
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
	m_pDepthBuffer = NULL;
	m_pShadowMapBuffer = NULL;

	ZeroMemory(&m_RenderTargets, sizeof(m_RenderTargets));
}

CascadedShadowMap::~CascadedShadowMap()
{
	if (m_pDepthTexture2D) m_pDepthTexture2D->Release();
	if (m_pShadowMap) m_pShadowMap->Release();
	if (m_pDepthTarget) m_pDepthTarget->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
	if (m_pDepthBuffer) m_pDepthBuffer->Release();
	if (m_pShadowMapBuffer) m_pShadowMapBuffer->Release();
}

void CascadedShadowMap::Init(UINT width, UINT height)
{
	HRESULT hr = S_OK;

	GameApp* app = GameApp::getInstance();

	D3D11_TEXTURE2D_DESC depthMapDesc;
	depthMapDesc.Width = width;
	depthMapDesc.Height = height;
	depthMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	depthMapDesc.ArraySize = MAX_CASCADED;
	depthMapDesc.Usage = D3D11_USAGE_DEFAULT;
	depthMapDesc.CPUAccessFlags = 0;
	depthMapDesc.MipLevels = 1;
	depthMapDesc.MiscFlags = 0;
	depthMapDesc.SampleDesc.Count = 1;
	depthMapDesc.SampleDesc.Quality = 0;

	hr = RENDER_CONTEXT::GetDevice()->CreateTexture2D(&depthMapDesc, NULL, &m_pDepthTexture2D);
	if (FAILED(hr))
	{
		abort();
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = depthMapDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for (int i = 0; i < MAX_CASCADED; i++)
	{
		m_RenderTargets[i] = new RENDER_TARGET(width, height, depthMapDesc.Format, true);
		m_RenderTargets[i]->_pTexture->_pD3DTexture = m_pDepthTexture2D;

		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = RENDER_CONTEXT::GetDevice()->CreateRenderTargetView(m_pDepthTexture2D, &rtvDesc, &(m_RenderTargets[i]->_RenderTarget));
		if (FAILED(hr))
		{
			abort();
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = depthMapDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = MAX_CASCADED;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateShaderResourceView(m_pDepthTexture2D, &srvDesc, &m_pShadowMap);
	if (FAILED(hr))
	{
		abort();
	}

	m_pDepthTarget = new RENDER_TARGET(width, height, DXGI_FORMAT_R32_TYPELESS, false);

	RENDER_CONTEXT::CreateVertexShader(const_cast<WCHAR*>(L"shaders/CascadedDepth_vs.cso"), &m_pVertexShader);
	RENDER_CONTEXT::CreatePixelShader(const_cast<WCHAR*>(L"shaders/CascadedDepth_ps.cso"), &m_pPixelShader);
	
	// Create the depth constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantDepthBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pDepthBuffer);

	// Create the shadow map constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantShadowMapBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pShadowMapBuffer);

	// Setup the viewport
	m_ViewPort.Width = (FLOAT)width;
	m_ViewPort.Height = (FLOAT)width;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;
}

void CascadedShadowMap::Update(
	const Camera& camera,
	const XMMATRIX& lightView,
	const XMMATRIX& lightProj)
{
	FLOAT nearZ = camera.GetNearZ();
	FLOAT farZ = camera.GetFarZ();
	FLOAT fovAngleY = camera.GetFLFovAngleY();
	FLOAT aspectRatio = camera.GetAspectRatio();
	XMMATRIX cascadedView = XMLoadFloat4x4(&camera.GetViewMatrix());

	// Calculate cascaded division line
	FLOAT cascaded[MAX_CASCADED + 1] = { 0.0f };
	for (int i = 0; i < MAX_CASCADED + 1; i++)
	{
		cascaded[i] = nearZ + (farZ - nearZ) * i / MAX_CASCADED;

		if (i > 0)
		{
			m_fCascadedDivisionEnd[i - 1] = cascaded[i];
		}
	}

	// Calculate subfrustum projection matrix
	XMMATRIX cascadedProj[MAX_CASCADED];
	for (int i = 0; i < MAX_CASCADED; i++)
	{
		cascadedProj[i] = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, cascaded[i], cascaded[i + 1]);
	}

	XMVECTOR vLightCameraOrthographicMin;
	XMVECTOR vLightCameraOrthographicMax;

	for (int iCascadedIndex = 0; iCascadedIndex < MAX_CASCADED; iCascadedIndex++)
	{
		XMMATRIX cameraViewMatrix = XMLoadFloat4x4(&camera.GetViewMatrix());
		XMVECTOR det;
		XMMATRIX matInverseViewCamera = XMMatrixInverse(&det, cameraViewMatrix);


        auto vProjection = XMLoadFloat4x4(&camera.GetProjMatrix());

		XMVECTOR boundsPoints[8];
		CreateFrustumPointsFromCascadeInterval(cascaded[iCascadedIndex], cascaded[iCascadedIndex + 1],
            vProjection, boundsPoints);

		vLightCameraOrthographicMin = g_vFLTMAX;
		vLightCameraOrthographicMax = g_vFLTMIN;

		for (int ibpIndex = 0; ibpIndex < 8; ibpIndex++)
		{
			// Transform the frustum from camera view space to world space.
			boundsPoints[ibpIndex] = XMVector4Transform(boundsPoints[ibpIndex], matInverseViewCamera);

			// Transform the point from world space to Light Camera Space.
			boundsPoints[ibpIndex] = XMVector4Transform(boundsPoints[ibpIndex], lightView);
 
			// Find the closest point.
			vLightCameraOrthographicMin = XMVectorMin(boundsPoints[ibpIndex], vLightCameraOrthographicMin);
			vLightCameraOrthographicMax = XMVectorMax(boundsPoints[ibpIndex], vLightCameraOrthographicMax);
		}

		/*XMMATRIX lightViewInverse = XMMatrixInverse(&det, lightView);

		XMVECTOR lookAt = vLightCameraOrthographicMin  + (vLightCameraOrthographicMax - vLightCameraOrthographicMin) / 2;
		XMVECTOR Eye = lookAt;
		Eye = XMVectorSetZ(Eye, 0.0f);
		lookAt = XMVector4Transform(lookAt, lightViewInverse);
		Eye = XMVector4Transform(Eye, lightViewInverse);

		auto upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		auto lookVector = XMVector3Normalize(lookAt - Eye);
		auto rightVector = XMVector3Normalize(XMVector3Cross(upVector, lookVector));
		upVector = XMVector3Normalize(XMVector3Cross(lookVector, rightVector));

		XMStoreFloat4x4(&m_mShadowView[iCascadedIndex], XMMatrixLookAtLH(Eye, lookAt, upVector));

		FLOAT width = max(
			XMVectorGetX(vLightCameraOrthographicMax - vLightCameraOrthographicMin),
			XMVectorGetY(vLightCameraOrthographicMax - vLightCameraOrthographicMin));

		XMStoreFloat4x4(
			&m_mShadowProj[iCascadedIndex], 
			XMMatrixOrthographicLH(
				width,
				width,
				0.01f,
				50));*/


		XMStoreFloat4x4(
			&m_mShadowProj[iCascadedIndex],
			XMMatrixOrthographicOffCenterLH(XMVectorGetX(vLightCameraOrthographicMin),
				XMVectorGetX(vLightCameraOrthographicMax),
				XMVectorGetY(vLightCameraOrthographicMin),
				XMVectorGetY(vLightCameraOrthographicMax),
				0.01f, 50));
	}

	XMStoreFloat4x4(&m_mShadowView, lightView);
}

void CascadedShadowMap::RenderToDepthMap(
	ID3D11DeviceContext* deviceContex,
	INT targetIndex)
{
	ConstantDepthBuffer cb;
	cb.mView = XMMatrixTranspose(XMLoadFloat4x4(&m_mShadowView));
	cb.mProjection = XMMatrixTranspose(XMLoadFloat4x4(&m_mShadowProj[targetIndex]));
	cb.vTextureSize = XMFLOAT4(1024, 1024, 0.0f, 0.0f);
	deviceContex->UpdateSubresource(m_pDepthBuffer, 0, NULL, &cb, 0, 0);

	// Set vertex shader
	deviceContex->VSSetShader(m_pVertexShader, NULL, 0);
	deviceContex->VSSetConstantBuffers(13, 1, &m_pDepthBuffer);

	// Set pixel shader
	deviceContex->PSSetShader(m_pPixelShader, NULL, 0);
	//deviceContex->PSSetConstantBuffers(13, 1, &m_pConstantBuffer);
}

void CascadedShadowMap::PrepareRenderWithShadowMap(ID3D11DeviceContext* deviceContex)
{
	ConstantShadowMapBuffer cb;
	ZeroMemory(&cb, sizeof(cb));
	cb.vTextureSize = XMFLOAT4(1024, 1024, 0.0f, 0.0f);
	cb.mView = XMMatrixTranspose(XMLoadFloat4x4(&m_mShadowView));
	memcpy(&cb.vfCascadeFrustumsEyeSpaceDepths, &m_fCascadedDivisionEnd[0], sizeof(m_fCascadedDivisionEnd));

	for (int i = 0; i < MAX_CASCADED; i++)
	{
		cb.mProjection[i] = XMMatrixTranspose(XMLoadFloat4x4(&m_mShadowProj[i]));
	}

	/*for (int i = 0; i < MAX_CASCADED; i++)
	{
		cb.mView[i] = XMMatrixTranspose(XMLoadFloat4x4(&m_mShadowView[i]));
	}*/

	deviceContex->UpdateSubresource(m_pShadowMapBuffer, 0, NULL, &cb, 0, 0);
	deviceContex->PSSetConstantBuffers(1, 1, &m_pShadowMapBuffer);
}

void CascadedShadowMap::SetAndClearRenderTargetView(
	ID3D11DeviceContext* deviceContex,
	INT targetIndex)
{
	RENDER_CONTEXT::SetCurrentRenderTarget(m_RenderTargets[targetIndex], 0);
	RENDER_CONTEXT::SetCurrentDepthTarget(m_pDepthTarget);
	RENDER_CONTEXT::ApplyRenderTargets();
	
	RENDER_CONTEXT::Clear(CF_CLEAR_COLOR | CF_CLEAR_ZBUFFER, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0);
}

void CascadedShadowMap::SetViewPort(ID3D11DeviceContext* deviceContex)
{
	deviceContex->RSSetViewports(1, &m_ViewPort);
}

void CascadedShadowMap::CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin,
	FLOAT fCascadeIntervalEnd,
	XMMATRIX &vProjection,
	XMVECTOR* pvCornerPointsWorld)
{

	XNA::Frustum vViewFrust;
	ComputeFrustumFromProjection(&vViewFrust, &vProjection);
	vViewFrust.Near = fCascadeIntervalBegin;
	vViewFrust.Far = fCascadeIntervalEnd;

	static const XMVECTORU32 vGrabY = { 0x00000000,0xFFFFFFFF,0x00000000,0x00000000 };
	static const XMVECTORU32 vGrabX = { 0xFFFFFFFF,0x00000000,0x00000000,0x00000000 };

	XMVECTORF32 vRightTop = { vViewFrust.RightSlope,vViewFrust.TopSlope,1.0f,1.0f };
	XMVECTORF32 vLeftBottom = { vViewFrust.LeftSlope,vViewFrust.BottomSlope,1.0f,1.0f };
	XMVECTORF32 vNear = { vViewFrust.Near,vViewFrust.Near,vViewFrust.Near,1.0f };
	XMVECTORF32 vFar = { vViewFrust.Far,vViewFrust.Far,vViewFrust.Far,1.0f };
	XMVECTOR vRightTopNear = XMVectorMultiply(vRightTop, vNear);
	XMVECTOR vRightTopFar = XMVectorMultiply(vRightTop, vFar);
	XMVECTOR vLeftBottomNear = XMVectorMultiply(vLeftBottom, vNear);
	XMVECTOR vLeftBottomFar = XMVectorMultiply(vLeftBottom, vFar);

	pvCornerPointsWorld[0] = vRightTopNear;
	pvCornerPointsWorld[1] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabX);
	pvCornerPointsWorld[2] = vLeftBottomNear;
	pvCornerPointsWorld[3] = XMVectorSelect(vRightTopNear, vLeftBottomNear, vGrabY);

	pvCornerPointsWorld[4] = vRightTopFar;
	pvCornerPointsWorld[5] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabX);
	pvCornerPointsWorld[6] = vLeftBottomFar;
	pvCornerPointsWorld[7] = XMVectorSelect(vRightTopFar, vLeftBottomFar, vGrabY);

}

VOID CascadedShadowMap::ComputeFrustumFromProjection(XNA::Frustum* pOut, XMMATRIX* pProjection)
{
	XMASSERT(pOut);
	XMASSERT(pProjection);

	// Corners of the projection frustum in homogenous space.
	static XMVECTOR HomogenousPoints[6] =
	{
		{ 1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
		{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
		{ 0.0f,  1.0f, 1.0f, 1.0f },   // top
		{ 0.0f, -1.0f, 1.0f, 1.0f },   // bottom

		{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
		{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
	};

	XMVECTOR Determinant;
	XMMATRIX matInverse = XMMatrixInverse(&Determinant, *pProjection);

	// Compute the frustum corners in world space.
	XMVECTOR Points[6];

	for (INT i = 0; i < 6; i++)
	{
		// Transform point.
		Points[i] = XMVector4Transform(HomogenousPoints[i], matInverse);
	}

	pOut->Origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
	pOut->Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// Compute the slopes.
	Points[0] = Points[0] * XMVectorReciprocal(XMVectorSplatZ(Points[0]));
	Points[1] = Points[1] * XMVectorReciprocal(XMVectorSplatZ(Points[1]));
	Points[2] = Points[2] * XMVectorReciprocal(XMVectorSplatZ(Points[2]));
	Points[3] = Points[3] * XMVectorReciprocal(XMVectorSplatZ(Points[3]));

	pOut->RightSlope = XMVectorGetX(Points[0]);
	pOut->LeftSlope = XMVectorGetX(Points[1]);
	pOut->TopSlope = XMVectorGetY(Points[2]);
	pOut->BottomSlope = XMVectorGetY(Points[3]);

	// Compute near and far.
	Points[4] = Points[4] * XMVectorReciprocal(XMVectorSplatW(Points[4]));
	Points[5] = Points[5] * XMVectorReciprocal(XMVectorSplatW(Points[5]));

	pOut->Near = XMVectorGetZ(Points[4]);
	pOut->Far = XMVectorGetZ(Points[5]);

	return;
}