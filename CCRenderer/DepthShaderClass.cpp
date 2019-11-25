#include "DepthShaderClass.h"
#include "d3dcompiler.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "commonh.h"

DepthShaderClass::DepthShaderClass()
{
	m_pTexture2D = NULL;
	m_pRenderTargetView = NULL;
	m_pShaderResourceView = NULL;
	m_pDepthStencil = NULL;
	m_pDepthStencilView = NULL;
	m_pConstantBuffer = NULL;
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
}

DepthShaderClass::~DepthShaderClass()
{
	if (m_pTexture2D) m_pTexture2D->Release();
	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pShaderResourceView) m_pShaderResourceView->Release();
	if (m_pDepthStencil) m_pDepthStencil->Release();
	if (m_pDepthStencilView) m_pDepthStencilView->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
}

HRESULT DepthShaderClass::Init(UINT width, UINT height)
{
	HRESULT hr;

	m_Width = width;

	m_Height = height;

	GameApp* app = GameApp::getInstance();

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = m_Width;
	textureDesc.Height = m_Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create render target texture
	hr = RENDER_CONTEXT::GetDevice()->CreateTexture2D(&textureDesc, NULL, &m_pTexture2D);
	if (FAILED(hr))
		return hr;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create render target view
	hr = RENDER_CONTEXT::GetDevice()->CreateRenderTargetView(m_pTexture2D, &renderTargetViewDesc, &m_pRenderTargetView);
	if (FAILED(hr))
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC shareResourceViewDesc;
	shareResourceViewDesc.Format = textureDesc.Format;
	shareResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shareResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shareResourceViewDesc.Texture2D.MipLevels = 1;

	// Create shader resource
	hr = RENDER_CONTEXT::GetDevice()->CreateShaderResourceView(m_pTexture2D, &shareResourceViewDesc, &m_pShaderResourceView);
	if (FAILED(hr))
		return hr;

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_Width;
	descDepth.Height = m_Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantDepthBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);

	RENDER_CONTEXT::CreateVertexShader(L"shaders/Depth_vs.cso", &m_pVertexShader);
	RENDER_CONTEXT::CreatePixelShader(L"shaders/Depth_ps.cso", &m_pPixelShader);

	// Setup the viewport
	m_ViewPort.Width = (FLOAT)m_Width;
	m_ViewPort.Height = (FLOAT)m_Height;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;

	return S_OK;
}

void DepthShaderClass::ResetViewPort(ID3D11DeviceContext* deviceContex)
{
	deviceContex->RSSetViewports(1, &m_ViewPort);
}

void DepthShaderClass::SetRenderTargetView( ID3D11DeviceContext* deviceContex )
{
	deviceContex->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
}

void DepthShaderClass::ClearRenderTargetView(
	ID3D11DeviceContext* deviceContex,
	FLOAT red,
	FLOAT green,
	FLOAT blue,
	FLOAT alpha)
{
	FLOAT clearColor[4] = { red, green, blue, alpha };

	// Clear the render target
	deviceContex->ClearRenderTargetView(m_pRenderTargetView, clearColor);

	// Clear the depth buffer to 1.0 (max depth)
	deviceContex->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DepthShaderClass::Render(
	ID3D11DeviceContext* deviceContex,
	const XMMATRIX& view,
	const XMMATRIX& projection,
	INT indexCount)
{
	ConstantDepthBuffer cb;
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	cb.vTextureSize = XMFLOAT4(m_Width, m_Height, 0.0f, 0.0f);
	deviceContex->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set vertex shader
	deviceContex->VSSetShader(m_pVertexShader, NULL, 0);
	deviceContex->VSSetConstantBuffers(13, 1, &m_pConstantBuffer);

	// Set pixel shader
	deviceContex->PSSetShader(m_pPixelShader, NULL, 0);
	deviceContex->PSSetConstantBuffers(13, 1, &m_pConstantBuffer);

	deviceContex->DrawIndexed(indexCount, 0, 0);
}