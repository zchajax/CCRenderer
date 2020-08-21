#include "Sphere.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "commonh.h"
#include <math.h>

#define PI 3.141592653589793

Sphere* Sphere::Create(const char* imagePath)
{
	Sphere* sphere = new Sphere();

	if (sphere)
	{
		HRESULT hr = sphere->Init(imagePath);

		if (FAILED(hr))
		{
			delete sphere;
			return nullptr;
		}
	}

	return sphere;
}

Sphere::Sphere()
{

}

Sphere::~Sphere()
{

}

HRESULT Sphere::CreateVertexIndicesBuffer()
{
	HRESULT hr = S_OK;

	int sliceCount = 30;
	int stackCount = 30;
	float radius = 1.0f;

	std::vector<Vertex> vertices;
	vertices.push_back(Vertex(XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f)));
	float phiStep = PI / stackCount;
	float thetaStep = 2.0f * PI / sliceCount;

	for (int i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (int j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;

			XMFLOAT3 p(
				(radius * sin(phi) * cos(theta)),
				(radius * cos(phi)),
				(radius * sin(phi) * sin(theta)));

			XMFLOAT3 n;
			XMStoreFloat3(&n, XMVector3Normalize(XMLoadFloat3(&p)));

			XMFLOAT2 uv(theta / (PI * 2), phi / PI);

			vertices.push_back(Vertex(p, n, uv));
		}
	}

	vertices.push_back(Vertex(XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)));

	std::vector<WORD> indices;

	for (int i = 1; i <= sliceCount; i++) 
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	int baseIndex = 1;

	int ringVertexCount = sliceCount + 1;

	for (int i = 0; i < stackCount - 2; i++) 
	{
		for (int j = 0; j < sliceCount; j++) 
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	int southPoleIndex = vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	m_pVertexBuffer = RENDER_CONTEXT::CreateVertexBuffer(vertices.size(), sizeof(Vertex), vertices.data());

	// Create indices buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices.data();
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	m_IndexCount = indices.size();

	return S_OK;
}