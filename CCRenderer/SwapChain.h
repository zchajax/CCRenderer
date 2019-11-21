#pragma once
#include <d3d11_1.h>

class RENDER_TARGET;
class SWAP_CHAIN
{
public:

	SWAP_CHAIN(HWND hWnd);

	~SWAP_CHAIN();

	void Present();

	RENDER_TARGET* GetFrontBuffer() { return m_pFrontBuffer; }

	RENDER_TARGET* GetDepthBuffer() { return m_pDepthBuffer; }

private:

	void Create();

	void Release();

private:

	IDXGISwapChain*	m_pSwapChain;
	RENDER_TARGET*	m_pFrontBuffer;
	RENDER_TARGET*	m_pDepthBuffer;
	HWND		    m_hWnd;
};
