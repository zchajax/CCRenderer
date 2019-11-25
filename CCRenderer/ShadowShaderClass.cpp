#include "ShadowShaderClass.h"
#include "GameApp.h"
#include "RenderContext.h"

ShadowShaderClass::ShadowShaderClass()
{
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
	m_pSamplerShadow = NULL;
}

ShadowShaderClass::~ShadowShaderClass()
{
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
	if (m_pSamplerShadow) m_pSamplerShadow->Release();
}

HRESULT ShadowShaderClass::Init()
{
	HRESULT hr;

	GameApp* app = GameApp::getInstance();

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = app->CompileShaderFromFile(L"shaders/Shdow.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA("The FX file cannot be compiled");
		return hr;
	}

	// Create the vertex shader
	hr = RENDER_CONTEXT::GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = app->CompileShaderFromFile(L"shaders/Shdow.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA("The FX file cannot be compiled");
		return hr;
	}

	// Create the pixel shader
	hr = RENDER_CONTEXT::GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;

	hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &m_pSamplerShadow);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void ShadowShaderClass::Render(
	ID3D11DeviceContext* context,
	ID3D11ShaderResourceView* depthTexture,
	INT indexCount)
{
	// Set vertex shader
	context->VSSetShader(m_pVertexShader, NULL, 0);

	// Set pixel shader
	context->PSSetShader(m_pPixelShader, NULL, 0);
	RENDER_CONTEXT::SetPixelShaderResource(1, depthTexture);
	RENDER_CONTEXT::SetPixelSampler(1, m_pSamplerShadow);

	// Draw
	context->DrawIndexed(indexCount, 0, 0);
}