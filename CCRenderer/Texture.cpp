#include "Texture.h"
#include "TextureManager.h"

HRESULT Texture::Create(const char* path, Texture** texture)
{
	Texture* temp = new Texture();
	
	HRESULT hr = temp->Init(path);
	if (FAILED(hr))
	{
		delete temp;
		return hr;
	}

	*texture = temp;

    return S_OK;
}

Texture::Texture()
{

}

Texture::~Texture()
{

}

HRESULT Texture::Init(const char* path)
{
	m_sPath = path;

	return TextureManager::GetInstance()->LoadTexture(path);
}

void Texture::Release()
{
	delete this;
}

ID3D11ShaderResourceView* Texture::GetShaderResoure()
{
	return TextureManager::GetInstance()->GetShaderResourceView(m_sPath.c_str());
}
