#pragma once

#include <d3d11_1.h>
#include <xnamath.h>

class SkyBox
{
public:

	SkyBox();

	~SkyBox();

	HRESULT Init(ID3D11Device* device);

	// Called per frame to update this object
	void Update(float delta);

	//Render this object
	void Render();

private:

	ID3D11Device*			m_pDevice;
	ID3D11VertexShader*		m_pVertexShader;
	ID3D11PixelShader*		m_pPixelShader;
	ID3D11InputLayout*		m_pVertexLayout;
	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;
	ID3D11Buffer*			m_pConstantBuffer;
	ID3D11ShaderResourceView*	m_pTextureRV;
	ID3D11SamplerState*		m_pSamplerLinear;
	ID3D11DepthStencilState*	m_pDepthStencilStateDisable;
	ID3D11DepthStencilState*	m_pDepthStencilStateAble;
	XMFLOAT4X4			m_World;
	XMFLOAT4X4			m_Wvp;
	XMFLOAT3			m_Position;

	XMFLOAT4X4			m_mViewMatrix[6];
	XMFLOAT4X4			m_mProjMatrix;
};
