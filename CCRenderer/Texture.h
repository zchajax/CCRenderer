#pragma once

#include <string>
#include <d3d11.h>

class Texture
{
public:

    static HRESULT Create(const char* path, Texture** texture);

	void Release();

	ID3D11ShaderResourceView* GetShaderResoure();

private:

	Texture();

	~Texture();

	HRESULT Init(const char* path);

	std::string m_sPath;
};
