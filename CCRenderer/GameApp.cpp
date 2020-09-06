#include "GameApp.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Texture_DX11.h"
#include "Scene.h"
#include "Cube.h"
#include "Sphere.h"
#include "Plane.h"
#include "Model.h"
#include "RenderDoc.h"
#include "EventDispatcher.h"
#include "CascadedShadowMap.h"
#include "TextureManager.h"
#include "SamplerStateManager.h"
#include "Rendering_Pipeline.h"
#include "commonh.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include <d3dcompiler.h>
#include <fstream>

#define FRAME_DELTA 0.0167f

GameApp* GameApp::s_GameApp = NULL;
      
#define ENABLE_DEBUG_DEVICE_CMD_ARGS "-debugdevice"
bool gEnableDebugDevice = false;
static inline void CheckDebugDeviceCmdArgs(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(ENABLE_DEBUG_DEVICE_CMD_ARGS, argv[i]) == 0)
		{
			gEnableDebugDevice = true;
			break;
		}
	}
}

#define ENABLE_MSAA_CMD_ARGS "-msaa"
bool gEnableMSAA = false;
static inline void CheckMsaaCmdArgs(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(ENABLE_MSAA_CMD_ARGS, argv[i]) == 0)
        {
            gEnableMSAA = true;
            break;
        }
    }
}

#define ENABLE_RENDERDOC_CMD_ARGS "-renderdoc"
bool gEnableRenderDoc = false;
static inline void CheckRenderDocCmdArgs(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(ENABLE_RENDERDOC_CMD_ARGS, argv[i]) == 0)
		{
			gEnableRenderDoc = true;
			break;
		}
	}
}

GameApp::GameApp(void)
{
	m_hWnd = NULL;
	m_pCascadedShadowMap = NULL;
}

GameApp::~GameApp(void)
{
}

GameApp* GameApp::getInstance()
{
	if (!s_GameApp)
	{
		s_GameApp = new GameApp();
	}

	return s_GameApp;
}

void GameApp::destroyInstance()
{
	if (s_GameApp)
	{
		delete s_GameApp;
	}
}

HRESULT GameApp::Init(HWND hWnd, int argc, char * argv[])
{
	m_hWnd = hWnd;

	CheckDebugDeviceCmdArgs(argc, argv);
    CheckMsaaCmdArgs(argc, argv);
	CheckRenderDocCmdArgs(argc, argv);

	if (gEnableRenderDoc)
	{
		RENDER_DOC::Initialize(gEnableDebugDevice);
	}

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	RENDER_CONTEXT::Init(hWnd, width, height);

	RENDERING_PIPELINE::Init();

	// Setup the viewport
	m_ViewPort.Width = (FLOAT)width;
	m_ViewPort.Height = (FLOAT)height;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;

	m_Scene = new Scene();
	m_Scene->Init();

	// Initialize camera
	m_Camera.Init();
	auto Eye = XMFLOAT4(9.39f, 7.44f, 7.48f, 1.0f);
	auto At = XMFLOAT4(8.74f, 7.08f, 6.81f, 1.0f);
	m_Camera.SetViewParams(Eye, At);
	m_Camera.SetProjParams(0.785f, width / (FLOAT)height, 0.01f, 100.0f);

	return S_OK;
}

int GameApp::Run()
{
	float deltaTime = 0.0f;

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Timer start
	m_Timer.Start();

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.tick();
			
			deltaTime += m_Timer.getDeltaTime();

			if (deltaTime > FRAME_DELTA)
			{
				// Update Scene
				GameApp::getInstance()->Update(deltaTime);

				// Render Scene
				GameApp::getInstance()->Render();

				deltaTime = 0.0f;
			}
			else
			{
				Sleep(0);
			}
		}
	}

	return (int)msg.wParam;
}

void GameApp::Update(float delta)
{
	// Update EventDispatcher
	EventDispatcher::getInstance()->Update(delta);

	// Update camera
	m_Camera.Update(delta);

	// Update Scene
	m_Scene->Update(delta);

	//// test
	//Node* test = m_Scene.at(0);

	//if (test)
	//{
	//	auto rotation = test->GetRotation();
	//	test->setRotation(XMFLOAT3(rotation.x, rotation.y + 1.0f * delta, rotation.z));
	//}
}

void GameApp::Render()
{
	RENDER_CONTEXT::Reset();

	RenderShadowMap();

	RenderScene();

	PostProcessing();

	// Flip 
	RENDER_CONTEXT::Present();
}

void GameApp::RenderShadowMap()
{
	m_Scene->RenderShadowMap();
}

void GameApp::RenderScene()
{
	RENDER_CONTEXT::PushMarker(MARK("BasePass"));

	RENDER_CONTEXT::GetImmediateContext()->RSSetViewports(1, &m_ViewPort);

    RENDER_CONTEXT::GetImmediateContext()->RSSetState(RENDER_CONTEXT::GetRasterizerState());

	//RENDER_CONTEXT::SetCurrentRenderTarget(RENDER_CONTEXT::GetFrontBuffer());
	RENDER_CONTEXT::SetCurrentDepthTarget(RENDER_CONTEXT::GetDepthBuffer());
	RENDERING_PIPELINE::SetTargetOutputColor();
	RENDERING_PIPELINE::SetTargetOutputDepth();
	RENDERING_PIPELINE::SetTargetOutputNormal();
	RENDER_CONTEXT::ApplyRenderTargets();

	RENDER_CONTEXT::Clear(CF_CLEAR_COLOR | CF_CLEAR_ZBUFFER, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0);

	// Render Camera
	m_Camera.Render();

	m_Scene->Render();

    extern bool gEnableMSAA;

    if (gEnableMSAA)
    {
        RENDERING_PIPELINE::Resolve();
    }

	RENDER_CONTEXT::PopMarker();
}

void GameApp::PostProcessing()
{
    RENDER_CONTEXT::PushMarker(MARK("Post-Processing"));
	RENDERING_PIPELINE::GammaCorrection();
    RENDER_CONTEXT::PopMarker();
}

void GameApp::Clear()
{
	if (m_pCascadedShadowMap)
	{
		delete m_pCascadedShadowMap;
		m_pCascadedShadowMap = NULL;
	}

	if (m_Scene)
	{
		delete m_Scene;

		m_Scene = NULL;
	}

	EventDispatcher::destroyInstance();
	FBXLoader::DestoryInstance();
	TextureManager::DestoryInstance();
	SamplerStateManager::DestoryInstance();

	RENDERING_PIPELINE::Release();

	RENDER_CONTEXT::Exit();
}

void GameApp::OnSize(int height, int width)
{
	m_Camera.SetAspectRatio(width / (FLOAT)height);
}

void GameApp::OnEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	EventDispatcher::getInstance()->OnEvent(hWnd, message, wParam, lParam);
}

HRESULT GameApp::CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return hr;
}

HRESULT GameApp::CreateShaderResourceViewFromFile(
	LPCWSTR                     pSrcFile,
	D3DX11_IMAGE_LOAD_INFO      *pLoadInfo,
	ID3DX11ThreadPump*          pPump,
	ID3D11ShaderResourceView**  ppShaderResourceView,
	HRESULT*                    pHResult)
{
	return D3DX11CreateShaderResourceViewFromFileW(RENDER_CONTEXT::GetDevice(), pSrcFile, pLoadInfo, pPump, ppShaderResourceView, pHResult);
}