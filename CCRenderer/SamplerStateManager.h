#pragma once
#include "SamplerState.h"
#include <map>

class SamplerStateManager
{
private:

	SamplerStateManager();

public:

	static SamplerStateManager* GetInstance()
	{
		if (!m_Instance)
		{
			m_Instance = new SamplerStateManager();
		}

		return m_Instance;
	}

	static void DestoryInstance()
	{
		if (m_Instance)
		{
			delete m_Instance;
			m_Instance = nullptr;
		}
	}

	~SamplerStateManager();

	SamplerState* GetSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE address, D3D11_COMPARISON_FUNC compFunc);

private:

	static SamplerStateManager* m_Instance;

	std::map<UINT64, SamplerState*> m_SamperStates;
};