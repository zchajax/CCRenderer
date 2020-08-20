#pragma once

#include <d3d11_1.h>
#include "commonh.h"
#include "xnacollision.h"
#include <DirectXMath.h>
using namespace DirectX;

class RENDER_TARGET;
class Camera;
class CascadedShadowMap
{
public:

	CascadedShadowMap();

	~CascadedShadowMap();

	void Init(UINT width, UINT height);

	void Update(
		const Camera& camera,
		const XMMATRIX& lightView,
		const XMMATRIX& lightProj);

	void RenderToDepthMap(
		ID3D11DeviceContext* deviceContex,
		INT targetIndex);

	void PrepareRenderWithShadowMap(ID3D11DeviceContext* deviceContex);

	void SetAndClearRenderTargetView(
		ID3D11DeviceContext* deviceContex,
		INT targetIndex);

	void SetViewPort(ID3D11DeviceContext* deviceContex);

	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin,
		FLOAT fCascadeIntervalEnd,
		XMMATRIX &vProjection,
		XMVECTOR* pvCornerPointsWorld);

	void ComputeFrustumFromProjection(XNA::Frustum* pOut, XMMATRIX* pProjection);

	ID3D11ShaderResourceView* getShadowMap()
	{
		return m_pShadowMap;
	}

private:

	D3D11_VIEWPORT				m_ViewPort;
	ID3D11Texture2D*			m_pDepthTexture2D;
	ID3D11ShaderResourceView*	m_pShadowMap;
	RENDER_TARGET*				m_RenderTargets[MAX_CASCADED];
	RENDER_TARGET*				m_pDepthTarget;
	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11Buffer*				m_pDepthBuffer;
	ID3D11Buffer*				m_pShadowMapBuffer;
	XMFLOAT4X4					m_mShadowView;
	XMFLOAT4X4					m_mShadowProj[MAX_CASCADED];
	FLOAT						m_fCascadedDivisionEnd[MAX_CASCADED];
};