#pragma once
#include <d3d11_1.h>

class TEXTURE_DX11
{
public:

	TEXTURE_DX11();
	
    TEXTURE_DX11(
        UINT32		uiWidth,
        UINT32		uiHeight,
        DXGI_FORMAT	eFormat,
        UINT32		nMipMapLevels,
        UINT8       sampleCount,
		BOOL		bRenderTarget	
	);

	~TEXTURE_DX11();

	void			            SetFilterType(D3D11_FILTER eFilterType) { _Filter = eFilterType; }
	void			            SetAddressUType(D3D11_TEXTURE_ADDRESS_MODE eAddressType) { _AddressU = eAddressType; }
	void			            SetAddressVType(D3D11_TEXTURE_ADDRESS_MODE eAddressType) { _AddressV = eAddressType; }

	ID3D11Texture2D*	        GetD3DTexture() { return _pD3DTexture; }
	ID3D11SamplerState*	        GetSamplerState() { return _pD3DSampler; }
	ID3D11ShaderResourceView*   GetShaderResource() { return _pShaderResourceView; }

	UINT32			            GetWidth() { return _uiWidth; }
    UINT32			            GetHeight() { return _uiHeight; }
    DXGI_FORMAT                 GetFormat() { return _Format; }
    UINT32                      GetMipMapLevels() { return _uiMipMapLevels; }

private:

	void			            Create();
	void			            Release();
	void			            CreateSampler();

private:

	UINT32			            _uiWidth;
	UINT32			            _uiHeight;
	DXGI_FORMAT		            _Format;
	UINT32			            _uiMipMapLevels;
    UINT8                       _uiSampleCount;

	D3D11_FILTER		        _Filter;
	D3D11_TEXTURE_ADDRESS_MODE  _AddressU;
	D3D11_TEXTURE_ADDRESS_MODE  _AddressV;
	FLOAT			            _MipLodBias;
	D3D11_COMPARISON_FUNC	    _ComparisionFunc;
    ID3D11Texture2D*            _pD3DTexture;
    ID3D11SamplerState*         _pD3DSampler;
    ID3D11ShaderResourceView*   _pShaderResourceView;
	
	BOOL			            _bRenderTarget;

	friend class		RENDER_TARGET;
	friend class		SWAP_CHAIN;
	friend class		CascadedShadowMap;
};
