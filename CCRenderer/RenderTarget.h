#pragma once
#include <d3d11_1.h>

class TEXTURE_DX11;
class RENDER_TARGET
{
public:

	RENDER_TARGET(
		UINT32		uiWidth,
		UINT32		uiHeight,
		DXGI_FORMAT	eFormat,
        UINT8       sampleCount,
		BOOL		isEmptyTarget
	);

	~RENDER_TARGET();

	ID3D11RenderTargetView* GetRenderTarget() { return _RenderTarget; }
	ID3D11DepthStencilView* GetDepthRenderTarget() { return _DepthRenderTarget; }
	TEXTURE_DX11*		GetTexture() { return _pTexture; }

	UINT32			GetWidth() { return _uiWidth; }
	UINT32			GetHeight() {  return _uiHeight; }
	DXGI_FORMAT		GetFormat() { return _Format; }
	BOOL			IsDepth() { return _bIsDepth; }

private:
	
	void			Create();
	void			Release();

private:

	ID3D11RenderTargetView*		_RenderTarget;
	ID3D11DepthStencilView*		_DepthRenderTarget;

	TEXTURE_DX11*		_pTexture;
	UINT32				_uiWidth;
	UINT32				_uiHeight;
	DXGI_FORMAT			_Format;
    UINT8               _SampleCount;
	BOOL				_bIsDepth;
	BOOL				_bIsEmptyTarget;

	friend class SWAP_CHAIN;
	friend class CascadedShadowMap;
};
