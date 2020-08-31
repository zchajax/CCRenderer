#include "SamplerState.h"
#include "RenderContext.h"
#include <assert.h>

SamplerState::SamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE address, D3D11_COMPARISON_FUNC compFunc)
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = filter;
	sampDesc.AddressU = address;
	sampDesc.AddressV = address;
	sampDesc.AddressW = address;
	sampDesc.ComparisonFunc = compFunc;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &m_SamplerState);
	assert(SUCCEEDED(hr));
}

SamplerState::~SamplerState()
{
	if (m_SamplerState)
	{
		m_SamplerState->Release();

		m_SamplerState = NULL;
	}
}