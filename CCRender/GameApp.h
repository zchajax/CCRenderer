#pragma once
#include <vector>
#include <d3d11_1.h>
#include <d3dx11.h>
#include <xnamath.h>
#include "Timer.h"
#include "Camera.h"
#include "SkyBox.h"
#include "LightSource.h"

class Node;
class CascadedShadowMap;
class SWAP_CHAIN;
class GameApp
{
public:

	GameApp(void);

	~GameApp();

	static GameApp* getInstance();

	static void destroyInstance();
	
	// Init GameApp
	HRESULT Init(HWND hWnd, int argc, char* argv[]);
	
	// Main Loop run
	int Run();

	// Called per frames
	void Update(float delta);

	//Render scene
	void Render();

	//Generate ShadowMap
	void RenderShadowMap();

	// normal pass rendering
	void RenderScene();

	// post-processing rendering
	void PostProcessing();

	// Clear GameApp
	void Clear();

	// Window size change
	void OnSize(int height, int width);

	// Recevied message
	void OnEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Add shape to the Rendering list
	void AddNote(Node* shape);

	// Remove shape from the rendering list
	void RemoveNode(Node* shape);

	// Get Main Camera
	Camera& getMainCamera()
	{
		return m_Camera;
	}

	// Compile shader from file
	HRESULT CompileShaderFromFile(
		LPCSTR szFileName,
		LPCSTR szEntryPoint,
		LPCSTR szShaderModel,
		ID3DBlob** ppBlobOut);

	HRESULT CreateShaderResourceViewFromFile(
		LPCSTR				pSrcFile,
		D3DX11_IMAGE_LOAD_INFO		*pLoadInfo,
		ID3DX11ThreadPump*		pPump,
		ID3D11ShaderResourceView**	ppShaderResourceView,
		HRESULT*			pHResult);

	wchar_t* AnsiToUnicode(const char* szStr)
	{
		int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
		if (nLen == 0)
		{
			return NULL;
		}
		wchar_t* pResult = new wchar_t[nLen];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
		return pResult;
	}

private:
	static GameApp*		s_GameApp;

	HWND			m_hWnd;
	D3D11_VIEWPORT		m_ViewPort;
	CascadedShadowMap*	m_pCascadedShadowMap;

	Timer			m_Timer;

	Camera			m_Camera;
	LightSource		m_LightSource;
	SkyBox			m_SkyBox;

	std::vector<Node*>	m_Nodes;
}
