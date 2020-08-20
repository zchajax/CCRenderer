#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
using namespace DirectX;

class DepthShaderClass
{
public:

	DepthShaderClass();

	~DepthShaderClass();

	HRESULT Init(UINT width, UINT height);

	void ResetViewPort(ID3D11DeviceContext* deviceContex);

	void SetRenderTargetView(
		ID3D11DeviceContext* deviceContex);

	void ClearRenderTargetView(
		ID3D11DeviceContext* deviceContex,
		FLOAT red,
		FLOAT green,
		FLOAT blue,
		FLOAT alpha);

	void Render(
		ID3D11DeviceContext* deviceContex,
		const XMMATRIX& view,
		const XMMATRIX& projection,
		INT indexCount);

	ID3D11ShaderResourceView* GetShaderResoureView()
	{
		return m_pShaderResourceView;
	}

private:

	D3D11_VIEWPORT				m_ViewPort;
	ID3D11Texture2D*			m_pTexture2D;
	ID3D11RenderTargetView*		m_pRenderTargetView;
	ID3D11ShaderResourceView*	m_pShaderResourceView;
	ID3D11Texture2D*			m_pDepthStencil;
	ID3D11DepthStencilView*		m_pDepthStencilView;
	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	FLOAT						m_Width;
	FLOAT						m_Height;
};