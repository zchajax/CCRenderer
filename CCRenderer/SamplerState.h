#pragma once
#include <d3d11_1.h>
#include <map>

class SamplerState
{
private:

	SamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE address, D3D11_COMPARISON_FUNC compFunc);

	~SamplerState();

public:

	ID3D11SamplerState* GetSamplerState()
	{
		return m_SamplerState;
	}

private:

	ID3D11SamplerState* m_SamplerState;

	friend class SamplerStateManager;
};