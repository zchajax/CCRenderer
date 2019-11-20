#pragma once

#include <d3d11_1.h>
#include <vector>
#include "RenderDefs.h"

#define MARK(T) L##T
typedef wchar_t MARK_TYPE;

class RENDER_TARGET;
class VERTEX_BUFFER;
class SWAP_CHAIN;

class RENDER_CONTEXT
{
public:
	
	static void			Init(HWND hWnd, UINT32 uiWidth, UINT32 uiHeight);
	static void			Exit();
	static void			Present();
	static void			Reset();

	static void			Clear(UINT32 flag, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT z, UINT8 s);

	static void			LoadShader(WCHAR* name, std::vector<char>& compiledShader, SIZE_T& size);
	static void			CreateVertexShader(WCHAR path, ID3D11VertexShader** pVertexShader);
	static void			CreateVertexShader(const void* bytes, SIZE_T size, ID3D11VertexShader** pVertexShader);
	static void			CreatePixelShader(WCHAR* path, ID3D11PixelShader** pPixelShader);

	static VERTEX_BUFFER*		CreateVertexBuffer(UINT32 vertexCount, UINT32 vertexStride, void* initData);
	static void			ReleaseVertexBuffer(VERTEX_BUFFER* buffer);

	static void			SetCurrentRenderTarget(RENDER_TARGET* renderTarget, UINT32 index = 0);
	static RENDER_TARGET*		GetCurrentRenderTarget(UINT32 index = 0);
	static void			SetCurrentDepthTarget(RENDER_TARGET* depthTarget);
	static RENDER_TARGET*		GetCurrentDepthTarget();
	static void			ApplyRenderTargets();

	static void			SetVertexShaderResource(UINT32 statSlot, ID3D11ShaderResourceView* shaderResource);
	static void			SetPixelShaderResource(UINT32 statSlot, ID3D11ShaderResourceView* shaderResource);
	static void			SetVertexSampler(UINT32 statSlot, ID3D11SamplerState* sampler);
	static void			SetPixelSampler(UINT32 statSlot, ID3D11SamplerState* sampler);

	static void			DrawPrimitive(D3D11_PRIMITIVE_TOPOLOGY type, UINT32 vertexCount, UINT32 startOffset);
	static void			DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY type, UINT32 indexCount, UINT32 startOffset);

	static void			PushMarker(const MARK_TYPE* marker);
	static void			PopMarker();

	static ID3D11Device*		GetDevice() { return _pD3DDevice; }
	static ID3D11DeviceContext*	GetImmediateContex() { return _pImmediateContex; }
	static RENDER_TARGET*		GetFrontBuffer();
	static RENDER_TARGET*		GetDepthBuffer();

	static UINT32			GetWidth() { return _uiWidth; }
	static UINT32			GetHeight() { return _uiHeight; }

private:

	static HWND			                _hWnd;
	static ID3D11Device*		        _pD3DDevice;
	static ID3D11DeviceContext*	        _pImmediateContex;
	static SWAP_CHAIN*		            _pSwapChain;

	static RENDER_TARGET*		        _pRenderTargets[RCBI_BUFFER_MAX];
	static RENDER_TARGET*		        _pDepthTarget;


	static ID3DUserDefinedAnnotation*   _pAnnotation;

	static UINT32 			            _uiWidth;
	static UINT32 			            _uiHeight;
};
