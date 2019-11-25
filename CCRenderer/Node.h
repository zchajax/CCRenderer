#pragma once

#include <d3d11_1.h>
#include <xnamath.h>

class VERTEX_BUFFER;
class Texture;
class Node
{
public:

	virtual ~Node();

	// Render this node to depth texture
	void RenderToDepthTexture();

	// Render this node with shadow map
	void RenderShadowMap();

	virtual void ApplyRenderState() = 0;

	// Called per frame to update this shape
	virtual void Update(float delta);

	// Draw Primitive
	virtual void Draw() = 0;

	// Set Position
	void SetPosition(const XMFLOAT3& pos)
	{
		m_IsDirty = true;

		m_Position = pos;
	}

	// Get Position
	const XMFLOAT3& GetPosition() const
	{
		return m_Position;
	}

	// Set Scale
	void setScale(const XMFLOAT3& scale)
	{
		m_IsDirty = true;

		m_Scale = scale;
	}

	// Get Scale
	const XMFLOAT3& GetScale() const
	{
		return m_Scale;
	}

	// Set Rotation
	void setRotation(const XMFLOAT3& rotation)
	{
		m_IsDirty = true;

		m_Rotation = rotation;
	}

	// Get Rotation
	const XMFLOAT3& GetRotation() const
	{
		return m_Rotation;
	}

	int GetIndexCount() const
	{
		return m_IndexCount;
	}

protected:

	Node();

	VERTEX_BUFFER*				m_pVertexBuffer;
	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11InputLayout*			m_pVertexLayout;
	ID3D11Buffer*				m_pIndexBuffer;
	ID3D11Buffer*				m_pConstantBuffer;
	Texture*					m_pTextureRV;
	ID3D11SamplerState*         m_pSamplerLinear;
	ID3D11SamplerState*         m_pSamplerShadow;
	XMFLOAT4X4					m_World;
	XMFLOAT4X4					m_Wvp;
	XMFLOAT3					m_Position;
	XMFLOAT3					m_Scale;
	XMFLOAT3					m_Rotation;
	INT							m_IndexCount;
	BOOL						m_IsDirty;
};