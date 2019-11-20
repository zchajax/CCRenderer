#include "Texture_DX11.h"
#include "RenderContext.h"
#include <assert.h>

TEXTURE_DX11::TEXTURE_DX11() : 
	_uiWidth(0),
	_uiHeight(0),
	_Format(DXGI_FORMAT_UNKNOWN),
	_uiMipMapLevels(1),
	_Filter(D3D11_FILTER_ANISOTROPIC),
	_AddressU(D3D11_TEXTURE_ADDRESS_WRAP),
	_AddressV(D3D11_TEXTURE_ADDRESS_WRAP),
	_MipLodBias(0.0f),
	_ComparisionFunc(D3D11_COMPARISON_NEVER),
	_pD3DTexture(nullptr),
	_pD3DSampler(nullptr),
	_pShaderResourceView(nullptr),
	_bRenderTarget(false)
{

}

TEXTURE_DX11::TEXTURE_DX11(
	UINT32		uiWidth,
	UINT32		uiHeight,
	DXGI_FORMAT	eFormat,
	UINT32		nMipMapLevels,
	BOOL		bRenderTarget
) : 
	_uiWidth(uiWidth),
	_uiHeight(uiHeight),
	_Format(eFormat),
	_uiMipMapLevels(nMipMapLevels),
	_Filter(D3D11_FILTER_ANISOTROPIC),
	_AddressU(D3D11_TEXTURE_ADDRESS_WRAP),
	_AddressV(D3D11_TEXTURE_ADDRESS_WRAP),
	_MipLodBias(0.0f),
	_ComparisionFunc(D3D11_COMPARISON_NEVER),
	_pD3DTexture(nullptr),
	_pD3DSampler(nullptr),
	_pShaderResourceView(nullptr),
	_bRenderTarget(bRenderTarget)
{

}

TEXTURE_DX11::~TEXTURE_DX11()
{
	Release();
}

void TEXTURE_DX11::Create()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = _uiWidth;
	desc.Height = _uiHeight;
	desc.MipLevels = _uiMipMapLevels;
	desc.ArraySize = 1;
	desc.Format = _Format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	if (_bRenderTarget)
	{
		if (_Format == DXGI_FORMAT_R32G8X24_TYPELESS || _Format == DXGI_FORMAT_R16_TYPELESS || _Format == DXGI_FORMAT_R32_TYPELESS)
		{
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		}

		else
		{
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		}
	}

	else
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}

	//Create texture
	HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateTexture2D(&desc, NULL, &_pD3DTexture);
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = _uiMipMapLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		if (desc.Format == DXGI_FORMAT_R16_TYPELESS)
		{
			srvDesc.Format = DXGI_FORMAT_R16_FLOAT;
		}
		else if (desc.Format == DXGI_FORMAT_R32_TYPELESS)
		{
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else
		{
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		}
	}
	else
	{
		srvDesc.Format = _Format;
	}

	//Create shader resource
	hr = RENDER_CONTEXT::GetDevice()->CreateShaderResourceView(_pD3DTexture, &srvDesc, &_pShaderResourceView);
	assert(SUCCEEDED(hr));
}

void TEXTURE_DX11::CreateSampler()
{
	D3D11_SAMPLER_DESC sampDesc;
	sampDesc.Filter = _Filter;
	sampDesc.AddressU = _AddressU;
	sampDesc.AddressV = _AddressV;
	sampDesc.AddressW = _AddressV;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = _ComparisionFunc;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = -D3D11_FLOAT32_MAX;
	sampDesc.MaxLOD = -D3D11_FLOAT32_MAX;

	HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &_pD3DSampler);
	assert(SUCCEEDED(hr));
}

void TEXTURE_DX11::Release()
{
	if (_pD3DTexture)
	{
		_pD3DTexture->Release();
	
		_pD3DTexture = nullptr;	
	}

	if (_pD3DSampler)
	{
		_pD3DSampler->Release();
		
		_pD3DSampler = nullptr;
	}

	if (_pShaderResourceView)
	{
		_pShaderResourceView->Release();
		
		_pShaderResourceView = nullptr;
	}	
}

	
