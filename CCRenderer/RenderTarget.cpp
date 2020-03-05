#include "RenderTarget.h"
#include "Texture_DX11.h"
#include "RenderContext.h"
#include <assert.h>

RENDER_TARGET::RENDER_TARGET(
    UINT32				uiWidth,
    UINT32				uiHeight,
    DXGI_FORMAT			eFormat,
    UINT8               sampleCount,
    BOOL				isEmptyTarget
) :
    _uiWidth(uiWidth),
    _uiHeight(uiHeight),
    _Format(eFormat),
    _SampleCount(sampleCount),
	_bIsEmptyTarget(isEmptyTarget),
	_bIsDepth(false),
	_pTexture(nullptr),
	_RenderTarget(nullptr),
	_DepthRenderTarget(nullptr)
{
	if (_bIsEmptyTarget)
	{
		_pTexture = new TEXTURE_DX11();
	}
	else
	{
		_pTexture = new TEXTURE_DX11(uiWidth, uiHeight, eFormat, 1, sampleCount, true);

		Create();
	}
}

RENDER_TARGET::~RENDER_TARGET()
{
	Release();
}

void RENDER_TARGET::Create()
{
	// depth render target
	if (_Format == DXGI_FORMAT_R32G8X24_TYPELESS || _Format == DXGI_FORMAT_R16_TYPELESS || _Format == DXGI_FORMAT_R32_TYPELESS)
	{
		_bIsDepth = true;

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = _Format;
		descDSV.Texture2D.MipSlice = 0;

        if (_SampleCount > 1)
        {
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        }

		if (_Format == DXGI_FORMAT_R16_TYPELESS)
		{
			descDSV.Format = DXGI_FORMAT_D16_UNORM;
		}
		else if (_Format == DXGI_FORMAT_R32_TYPELESS)
		{
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		}
		else
		{
			descDSV.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		}

		HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateDepthStencilView(_pTexture->GetD3DTexture(), &descDSV, &_DepthRenderTarget);
		assert(SUCCEEDED(hr));
	}

	// colored render target
	else
	{
		_bIsDepth = false;

		D3D11_RENDER_TARGET_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = _Format;
		descDSV.Texture2D.MipSlice = 0;

        if (_SampleCount > 1)
        {
            descDSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            descDSV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        }

		HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateRenderTargetView(_pTexture->GetD3DTexture(), &descDSV, &_RenderTarget);
		assert(hr == S_OK);
	}
}

void RENDER_TARGET::Release()
{
	if (_pTexture)
	{
		if (_bIsEmptyTarget)
		{
			_pTexture->Release();
		}

		_pTexture = nullptr;
	}

	if (_RenderTarget)
	{
		_RenderTarget->Release();

		_RenderTarget = nullptr;
	}

	if (_DepthRenderTarget)
	{
		_DepthRenderTarget->Release();

		_DepthRenderTarget = nullptr;
	}
}