#include "TextureManager.h"
#include "GameApp.h"

TextureManager* TextureManager::m_pInstance = NULL;
extern const wchar_t* GetWC(const char* c);

TextureManager::TextureManager()
{
	
}

TextureManager::~TextureManager()
{
	for (auto texture : m_mTextures)
	{
		texture.second->Release();
	}
	m_mTextures.clear();
}

HRESULT TextureManager::LoadTexture(const char* path)
{
	HRESULT hr = S_OK;
	
	std::map<std::string, ID3D11ShaderResourceView*>::iterator iter;
	if ((iter = m_mTextures.find(path)) != m_mTextures.end())
	{
		return S_OK;
	}

	GameApp* app = GameApp::getInstance();
	
	ID3D11ShaderResourceView* tempTexture = NULL;
	hr = app->CreateShaderResourceViewFromFile(GetWC(path), NULL, NULL, &tempTexture, NULL);
	if (FAILED(hr))
	{
		return hr;
	}	

	m_mTextures[path] = tempTexture;

	return S_OK;
}

ID3D11ShaderResourceView* TextureManager::GetShaderResourceView(const char* path)
{
	ID3D11ShaderResourceView* temp = NULL;

	if (m_mTextures.find(path) != m_mTextures.end())
	{
		temp = 	m_mTextures[path];
	}

	return temp;
}
