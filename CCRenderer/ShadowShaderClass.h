#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
using namespace DirectX;

class ShadowShaderClass
{
public:

	ShadowShaderClass();

	~ShadowShaderClass();

	HRESULT Init();

	void Render(
		ID3D11DeviceContext* context,
		ID3D11ShaderResourceView* depthTexture,
		INT indexCount);

private:

	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11SamplerState*         m_pSamplerShadow;
};
