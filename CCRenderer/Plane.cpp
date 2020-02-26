#include "Plane.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "commonh.h"

Plane* Plane::Create(const char* imagePath)
{
	Plane* plane = new Plane();

	if (plane)
	{
		HRESULT hr = plane->Init(imagePath);

        if (FAILED(hr))
        {
            delete plane;

            return nullptr;
        }
	}

	return plane;
}

Plane::Plane()
{

}

Plane::~Plane()
{

}

HRESULT Plane::CreateVertexIndicesBuffer()
{
	HRESULT hr = S_OK;

	// Create vertex buffer 
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
	};

	m_pVertexBuffer = RENDER_CONTEXT::CreateVertexBuffer(4, sizeof(Vertex), vertices);

	// Create indices buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	m_IndexCount = 6;

	return S_OK;
}
