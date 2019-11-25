#include "RenderContext.h"
#include "RenderTarget.h"
#include "SwapChain.h"
#include "VertexBuffer.h"
#include <fstream>
#include <assert.h>

HWND RENDER_CONTEXT::_hWnd = 0;
ID3D11Device* RENDER_CONTEXT::_pD3DDevice = nullptr;
ID3D11DeviceContext* RENDER_CONTEXT::_pImmediateContext = nullptr;
SWAP_CHAIN* RENDER_CONTEXT::_pSwapChain = nullptr;
ID3DUserDefinedAnnotation* RENDER_CONTEXT::_pAnnotation = nullptr;
UINT32 RENDER_CONTEXT::_uiWidth = 0;
UINT32 RENDER_CONTEXT::_uiHeight = 0;
RENDER_TARGET* RENDER_CONTEXT::_pRenderTargets[] = {0};
RENDER_TARGET* RENDER_CONTEXT::_pDepthTarget = NULL;

void RENDER_CONTEXT::Init(HWND hWnd, UINT32 uiWidth, UINT32 uiHeight)
{
	_hWnd = hWnd;
	_uiWidth = uiWidth;
	_uiHeight = uiHeight;

	D3D_FEATURE_LEVEL tFeatureLevel;
	HRESULT res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, 0, D3D11_SDK_VERSION, NULL, &tFeatureLevel, NULL);
	if (tFeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		//MessageBox(hWnd, L"Your graphics card not support dx11");
		exit(0);
	}

	D3D11_CREATE_DEVICE_FLAG flags = (D3D11_CREATE_DEVICE_FLAG)(0);
	extern bool gEnableDebugDevice;
	if (gEnableDebugDevice)
	{
		flags = static_cast<D3D11_CREATE_DEVICE_FLAG>(flags | D3D11_CREATE_DEVICE_DEBUG);
	}

	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &_pD3DDevice, &tFeatureLevel, &_pImmediateContext);
	if (!_pD3DDevice)
	{
		//MessageBox(hWnd, L"Your graphics card not support dx11");
		exit(0);
	}

	HRESULT hr = _pImmediateContext->QueryInterface(__uuidof(_pAnnotation), reinterpret_cast<void**>(&_pAnnotation));
	assert(SUCCEEDED(hr));

	_pSwapChain = new SWAP_CHAIN(hWnd);
}

void RENDER_CONTEXT::Exit()
{
	if (_pSwapChain)
	{
		delete _pSwapChain;
		
		_pSwapChain = NULL;
	}

	if (_pImmediateContext)
	{
		_pImmediateContext->Release();
		_pImmediateContext = NULL;
	}

	if (_pD3DDevice)
	{
		_pD3DDevice->Release();

		_pD3DDevice = NULL;
	}
	
}

void RENDER_CONTEXT::PushMarker(const MARK_TYPE* marker)
{
	if (_pAnnotation)
	{
		_pAnnotation->BeginEvent(marker);
	}
}

void RENDER_CONTEXT::PopMarker()
{
	if (_pAnnotation)
	{
		_pAnnotation->EndEvent();
	}
}

void RENDER_CONTEXT::Present()
{
	if (_pSwapChain)
	{
		_pSwapChain->Present();
	}
}

void RENDER_CONTEXT::Clear(UINT32 flag, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT z, UINT8 s)
{
	if (flag & CF_CLEAR_COLOR)
	{
		FLOAT clearColor[4] = {r, g, b, a};
		if (_pRenderTargets[RCBI_COLOR_BUFFER])
		{
			_pImmediateContext->ClearRenderTargetView(_pRenderTargets[RCBI_COLOR_BUFFER]->GetRenderTarget(), clearColor);
		}

		FLOAT clearNormal[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		if (_pRenderTargets[RCBI_NORMAL_BUFFER])
		{
			_pImmediateContext->ClearRenderTargetView(_pRenderTargets[RCBI_NORMAL_BUFFER]->GetRenderTarget(), clearNormal);
		}

		FLOAT clearDepth[4] = {z, z, z, z};
		if (_pRenderTargets[RCBI_DEPTH_BUFFER])
		{
            _pImmediateContext->ClearRenderTargetView(_pRenderTargets[RCBI_DEPTH_BUFFER]->GetRenderTarget(), clearDepth);
		}
	}

	if (flag & (CF_CLEAR_ZBUFFER | CF_CLEAR_STENCIL))
	{
		UINT clearFlag = 0;

		if (flag & CF_CLEAR_ZBUFFER)
		{
			clearFlag |= D3D11_CLEAR_DEPTH;
		}

		if (flag & CF_CLEAR_STENCIL)
		{
			clearFlag |= D3D11_CLEAR_STENCIL;
		}

		_pImmediateContext->ClearDepthStencilView(_pDepthTarget->GetDepthRenderTarget(), clearFlag, z, s);
	}	
}

void RENDER_CONTEXT::LoadShader(const WCHAR* name, std::vector<char>& compiledShader, SIZE_T& size)
{
	std::ifstream fin(name, std::ifstream::in | std::ifstream::binary);

	if (!fin.good())
	{
		return;
	}

	fin.seekg(0, std::ios::end);
	size = (int)fin.tellg();
	fin.seekg(0, std::ios::beg);
	compiledShader.clear();
	compiledShader.resize(size);
	fin.read(&compiledShader[0], size);
	fin.close();
}

void RENDER_CONTEXT::CreateVertexShader(WCHAR* path, ID3D11VertexShader** pVertexShader)
{
	std::vector<char> compiledShader;
	SIZE_T size = 0;
    LoadShader(path, compiledShader, size);
	if (!size)
	{
		return;
	}

	RENDER_CONTEXT::GetDevice()->CreateVertexShader(&compiledShader[0], size, NULL, pVertexShader);
}

void RENDER_CONTEXT::CreateVertexShader(const void* bytes, SIZE_T size, ID3D11VertexShader** pVertexShader)
{
	RENDER_CONTEXT::GetDevice()->CreateVertexShader(bytes, size, NULL, pVertexShader);
}

void RENDER_CONTEXT::CreatePixelShader(const WCHAR* path, ID3D11PixelShader** pPixelShader)
{
	// Load the pixel shader file
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	LoadShader(path, compiledShader, size);
	if (!size)
	{
		return;
	}
	
	RENDER_CONTEXT::GetDevice()->CreatePixelShader(&compiledShader[0], size, NULL, pPixelShader);
}

void RENDER_CONTEXT::ReleaseVertexBuffer(VERTEX_BUFFER* buffer)
{
	if (buffer)
	{
		delete buffer;
	}
}

VERTEX_BUFFER* RENDER_CONTEXT::CreateVertexBuffer(UINT32 vertexCount, UINT32 vertexStride, void* initData)
{
	return new VERTEX_BUFFER(vertexCount, vertexStride, initData);
}

void RENDER_CONTEXT::SetCurrentRenderTarget(RENDER_TARGET* renderTarget, UINT32 index)
{
	_pRenderTargets[index] = renderTarget;
}

RENDER_TARGET* RENDER_CONTEXT::GetCurrentRenderTarget(UINT32 index)
{
	return _pRenderTargets[index];
}

void RENDER_CONTEXT::SetCurrentDepthTarget(RENDER_TARGET* depthTarget)
{
	_pDepthTarget = depthTarget;
}

RENDER_TARGET* RENDER_CONTEXT::GetCurrentDepthTarget()
{
	return _pDepthTarget;
}

void RENDER_CONTEXT::ApplyRenderTargets()
{
	ID3D11RenderTargetView* pRenderTargets[RCBI_BUFFER_MAX];
	ZeroMemory(pRenderTargets, sizeof(pRenderTargets));

	ID3D11DepthStencilView* pDepthRenderTarget = NULL;

	for (int i = 0; i < RCBI_BUFFER_MAX; i++)
	{
		if (_pRenderTargets[i])
		{
			pRenderTargets[i] = _pRenderTargets[i]->GetRenderTarget();
		}
	}

	if (_pDepthTarget)
	{
		pDepthRenderTarget = _pDepthTarget->GetDepthRenderTarget();
	}

	RENDER_CONTEXT::GetImmediateContext()->OMSetRenderTargets(RCBI_BUFFER_MAX, pRenderTargets, pDepthRenderTarget);
}

void RENDER_CONTEXT::SetVertexShaderResource(UINT32 statSlot, ID3D11ShaderResourceView* shaderResource)
{
	_pImmediateContext->VSSetShaderResources(statSlot, 1, &shaderResource);
}

void RENDER_CONTEXT::SetPixelShaderResource(UINT32 statSlot, ID3D11ShaderResourceView* shaderResource)
{
	_pImmediateContext->PSSetShaderResources(statSlot, 1, &shaderResource);
}

void RENDER_CONTEXT::SetVertexSampler(UINT32 statSlot, ID3D11SamplerState* sampler)
{
	_pImmediateContext->VSSetSamplers(statSlot, 1, &sampler);
}

void RENDER_CONTEXT::SetPixelSampler(UINT32 statSlot, ID3D11SamplerState* sampler)
{
	_pImmediateContext->PSSetSamplers(statSlot, 1, &sampler);
}

void RENDER_CONTEXT::DrawPrimitive(D3D11_PRIMITIVE_TOPOLOGY type, UINT32 vertexCount, UINT32 startOffset)
{
	_pImmediateContext->IASetPrimitiveTopology(type);

	_pImmediateContext->Draw(vertexCount, startOffset);
}

void RENDER_CONTEXT::DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY type, UINT32 indexCount, UINT32 startOffset)
{
	_pImmediateContext->IASetPrimitiveTopology(type);

	_pImmediateContext->DrawIndexed(indexCount, startOffset, 0);
}

RENDER_TARGET* RENDER_CONTEXT::GetFrontBuffer()
{
	if (_pSwapChain)
	{
		return _pSwapChain->GetFrontBuffer();
	}

	return NULL;
}

RENDER_TARGET* RENDER_CONTEXT::GetDepthBuffer()
{
	if (_pSwapChain)
	{
		return _pSwapChain->GetDepthBuffer();
	}
	
	return NULL;
}
