#include "SwapChain.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Texture_DX11.h"
#include <assert.h>

SWAP_CHAIN::SWAP_CHAIN(HWND hWnd) : 
	m_hWnd(hWnd),
	m_pSwapChain(nullptr),
	m_pFrontBuffer(nullptr),
	m_pDepthBuffer(nullptr)
{
	Create();
}

SWAP_CHAIN::~SWAP_CHAIN()
{
	Release();
}

void SWAP_CHAIN::Create()
{
	HRESULT hr;
	
	IDXGIFactory1* dxgiFactory = nullptr;
	IDXGOAdapter* dxgiDevice = nullptr;
	hr = RENDER_CONTEXT::GetDevice()->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	if (SUCCEEDED(hr))
	{
		IDXGIAdapater* adapter = nullptr;
		hr = dxgiDevice->GetAdapter(&adapter);
		if (SUCCEEDED(hr))
		{
			hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
			adapter->Release();
		}
		dxgiDevice->Release();
	}

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = RENDER_CONTEXT::GetWidth();
	sd.BufferDesc.Height = RENDER_CONTEXT::GetHeight();
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	sd.OutputWindow = m_Wind;
	sd.SampledDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITH;
	sd.Windowed = true;

	hr = dxgiFactory->CreateSwapChain(RENDER_CONTEXT::GetDevice(), &sd, &m_pSwapChain);
	assert(SUCCEEDED(hr));

	m_pFrontBuffer = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_B8GBR8A8_UNORM, true);
	m_pFrontBuffer->pTexture->_uiWidth = RENDER_CONTEXT::GetWidth();
	m_pFrontBuffer->pTexture->_uiHeight = RENDER_CONTEXT::GetHeight();
	m_pFrontBuffer->pTexture->_Format = DXGI_FORMAT_B8GBR8A8_UNORM;

	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_pFrontBuffer->_pTexture->pD3DTexture);
	assert(SUCCEEDED(hr));

	// create colored render target
	hr = RENDER_CONTEXT::GetDevice()->CreateRenderTargetView(p_FrontBuffer->_pTexture->_pD3DTexture, nullptr, &m_pFrontBuffer->_RenderTarget);
	assert(SUCCEEDED(hr));

	// create depth render target
	m_pDepthBuffer = new RenderTarget(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_R32G8X24_TYPELESS, false);
	assert(SUCCEEDED(hr));
	
}

void SWAP_CHAIN::Release()
{
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = NULL;
	}
}

void SWAP_CHAIN::Present()
{	
	m_pSwapChain->Present(0, 0);
}
