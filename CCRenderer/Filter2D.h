#pragma once

#include <d3d11_1.h>

class VERTEX_BUFFER;
class TEXTURE_DX11;
class RENDER_TARGET;
class FILTER2D
{
public:

	FILTER2D(const WCHAR* pixelShaderPath);

	~FILTER2D();

	void Init(TEXTURE_DX11* source, RENDER_TARGET* target);

	void Apply();

private:

	VERTEX_BUFFER*				m_pVertexBuffer;
	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11InputLayout*			m_pVertexLayout;
};