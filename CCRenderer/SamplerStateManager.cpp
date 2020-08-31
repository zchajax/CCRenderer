#include "SamplerStateManager.h"

SamplerStateManager* SamplerStateManager::m_Instance = NULL;

SamplerStateManager::SamplerStateManager()
{

}

SamplerStateManager::~SamplerStateManager()
{
	for (auto texture : m_SamperStates)
	{
		delete texture.second;
	}
	m_SamperStates.clear();
}

SamplerState* SamplerStateManager::GetSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE address, D3D11_COMPARISON_FUNC compFunc)
{
	UINT64 hash =  filter | address << 8 | compFunc << 16;

	auto iter = m_SamperStates.find(hash);

	if (iter != m_SamperStates.end())
	{
		return m_SamperStates[hash];
	}

	auto samplerState = new SamplerState(filter, address, compFunc);
	m_SamperStates[hash] = samplerState;

	return samplerState;
}

