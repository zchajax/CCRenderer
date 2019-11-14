#pragma once

#include <d3d11_1.h>
#include <map>

class TextureManager
{
public:
	static TextureManager* GetInstance()
	{
		if (!m_pInstane)
		{
			m_pInstance = new TextureManager();
		}
		return m_pInstance;
	}

	static void DestoryInstance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = null;
		}
	}

	HRESULT LoadTexture(const char* path);

	ID3DShaderResourceView* GetShaderResourceView(const char* path);

private:
	
	TextureManager();
	
	~TextureManager();

	static TextureManager* m_pInstance;

	std::map<std:string, ID3D11ShaderResourceView*> m_mTextures;
};
