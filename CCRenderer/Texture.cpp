#include "Texture.h"
#include "TextureManager.h"

extern bool IsFileExists(const std::string& name);

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
    if (IsFileExists(path))
    {
        m_sPath = path;
    }
    else
    {
        m_sPath = "assets/default.bmp";
    }

    return TextureManager::GetInstance()->LoadTexture(m_sPath.c_str());
}

void Texture::Release()
{
	delete this;
}

ID3D11ShaderResourceView* Texture::GetShaderResoure()
{
	return TextureManager::GetInstance()->GetShaderResourceView(m_sPath.c_str());
}
