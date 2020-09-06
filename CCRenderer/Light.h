#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
using namespace DirectX;

class Light
{
public:

	enum LIGHT_TYPE
	{
		DIRECTIONAL = 0,
		POINT,
	};

	Light();

	virtual ~Light();

	virtual void Update(float delta);

	virtual void Apply() = 0;

	void SetPosition(XMFLOAT4& position)
	{
		m_Position = position;
	}

	const XMFLOAT4& GetPosition() const
	{
		return m_Position;
	}

	void SetColor(XMFLOAT4& color)
	{
		m_Color = color;
	}

	const XMFLOAT4& GetColor() const
	{
		return m_Color;
	}

	void SetIntensity(FLOAT intensity)
	{
		m_Intensity = intensity;
	}

	FLOAT GetIntensity() const
	{
		return m_Intensity;
	}

	void SetType(LIGHT_TYPE type)
	{
		m_Type = type;
	}

	LIGHT_TYPE GetType() const
	{
		return m_Type;
	}

protected:

	XMFLOAT4 m_Position;

	XMFLOAT4 m_Color;

	FLOAT m_Intensity;

	LIGHT_TYPE m_Type;

	ID3D11Buffer* m_pConstantBuffer;
};