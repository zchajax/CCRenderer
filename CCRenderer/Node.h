#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Light.h"
using namespace DirectX;

class SamplerState;
class VERTEX_BUFFER;
class Texture;
class Node
{
public:

	enum NODE_TYPE
	{
		NODE_TYPE_OPAQUE = 0,
		NODE_TYPE_TRANSPARENT
	};

	virtual ~Node();

	// Render this node to depth texture
	void RenderToDepthTexture();

	// Render this node with shadow map
	void Render();

	virtual void ApplyRenderState() = 0;

	// Called per frame to update this shape
	virtual void Update(float delta);

	// Draw Primitive
	virtual void Draw() = 0;

	// Set Type
	void SetType(NODE_TYPE type)
	{
		m_Type = type;
	}

	NODE_TYPE GetType()
	{
		return m_Type;
	}

	// Get Type

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

	// Set Color
	void SetColor(const XMFLOAT4& color)
	{
		m_Color = color;
	}

	const XMFLOAT4& GetColor() const
	{
		return m_Color;
	}

	// Get Color;

	void SetDistanceFromCamera(float distance)
	{
		m_DistanceFromCamera = distance;
	}

	FLOAT GetDistanceFromCamera()
	{
		return m_DistanceFromCamera;
	}

	const XMFLOAT4X4& GetWorld() const
	{
		return m_World;
	}

	int GetIndexCount() const
	{
		return m_IndexCount;
	}

	void SetBlendAdd()
	{
		m_BlendStateCurrent = m_BlendStateAdd;
	}

	void SetBlendTransparent()
	{
		m_BlendStateCurrent = m_BlendStateTransparent;
	}

	void EnableDepthWrite()
	{
		m_pStencilStateCurrent = m_pStencilStateOpaque;
	}

	void DisableDepthWrite()
	{
		m_pStencilStateCurrent = m_pStencilStateTransparent;
	}

	void SetRenderLightType(Light::LIGHT_TYPE type)
	{
		m_RenderLightType = type;
	}

	HRESULT InitNode();

protected:

	Node();

	NODE_TYPE					m_Type;

	VERTEX_BUFFER*				m_pVertexBuffer;
	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShaderPoint;
	ID3D11PixelShader*			m_pPixelShaderDirecional;
	ID3D11InputLayout*			m_pVertexLayout;
	ID3D11Buffer*				m_pIndexBuffer;
	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11Buffer*				m_pMaterialBuffer;
	Texture*					m_pTextureRV;
	XMFLOAT4					m_Color;
	SamplerState*				m_pSamplerLinear;
	SamplerState*				m_pSamplerShadow;
	ID3D11DepthStencilState*	m_pStencilStateOpaque;
	ID3D11DepthStencilState*	m_pStencilStateTransparent;
	ID3D11DepthStencilState*	m_pStencilStateCurrent;
	ID3D11BlendState*			m_BlendStateAdd;
	ID3D11BlendState*			m_BlendStateTransparent;
	ID3D11BlendState*			m_BlendStateCurrent;
	XMFLOAT4X4					m_World;
	XMFLOAT4X4					m_Wvp;
	XMFLOAT3					m_Position;
	XMFLOAT3					m_Scale;
	XMFLOAT3					m_Rotation;
	FLOAT						m_DistanceFromCamera;
	INT							m_IndexCount;
	BOOL						m_IsDirty;
	Light::LIGHT_TYPE			m_RenderLightType;
};