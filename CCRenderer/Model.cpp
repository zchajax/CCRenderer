#include "Model.h"
#include "Texture.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "commonh.h"
#include <memory>

Model* Model::Create(const char* filename)
{
	Model* model = new Model();

	if (model)
	{
		model->Init(filename);
	}

	return model;
}

Model::Model()
{
	m_pNormalMap = NULL;
	m_pMetallicSmoothMap = NULL;
	m_pOcclusionMap = NULL;
}

Model::~Model()
{
	if (m_pNormalMap) m_pNormalMap->Release();
	if (m_pMetallicSmoothMap) m_pMetallicSmoothMap->Release();
	if (m_pOcclusionMap) m_pOcclusionMap->Release();
}

HRESULT Model::Init(const char* filename)
{
	HRESULT hr;

	// Create fbx data
	hr = FBXLoader::GetInstance()->LoadFBX(filename, &m_MeshNode);

	if (FAILED(hr))
	{
		return hr;
	}

	// Create the vertex shader
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	RENDER_CONTEXT::LoadShader(L"shaders/Model_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(L"shaders/Model_ps.cso", &m_pPixelShader);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = RENDER_CONTEXT::GetDevice()->CreateInputLayout(layout, numElements, &compiledShader[0],
		size, &m_pVertexLayout);
	if (FAILED(hr))
		return hr;

	// Create vertex buffer 
	int vertexCount = m_MeshNode.m_positionArray.size();
	std::unique_ptr<VertexTBN[]> vertices(new VertexTBN[vertexCount]);
	
	for (int i = 0; i < vertexCount; ++i)
	{
		auto pos = m_MeshNode.m_positionArray.at(i);
		vertices[i].Pos = XMFLOAT3((FLOAT)pos.mData[0],
			(FLOAT)pos.mData[1],
			(FLOAT)pos.mData[2]);

		auto normal = m_MeshNode.m_normalArray.at(i);
		vertices[i].Normal = XMFLOAT3((FLOAT)normal.mData[0],
			(FLOAT)normal.mData[1],
			(FLOAT)normal.mData[2]);

		auto tangent = m_MeshNode.m_tangentArray.at(i);
		vertices[i].Tangent = XMFLOAT3((FLOAT)tangent.mData[0],
			(FLOAT)tangent.mData[1],
			(FLOAT)tangent.mData[2]);

		auto binormal = m_MeshNode.m_binormaArray.at(i);
		vertices[i].BiNormal = XMFLOAT3((FLOAT)binormal.mData[0],
			(FLOAT)binormal.mData[1],
			(FLOAT)binormal.mData[2]);

		auto texCoord = m_MeshNode.m_texcoordArray.at(i);
		vertices[i].Tex = XMFLOAT2((FLOAT)(abs)(texCoord.mData[0]),
			(FLOAT)abs(1.0f - texCoord.mData[1]));
	}

	m_pVertexBuffer = RENDER_CONTEXT::CreateVertexBuffer(vertexCount, sizeof(VertexTBN), vertices.get());

	// Create indices buffer
	int indexCount = m_MeshNode.indexArray.size();
	std::unique_ptr<WORD[]> indices(new WORD[indexCount]);
	for (int i = 0; i < indexCount; i++)
	{
		indices[i] = (WORD)m_MeshNode.indexArray.at(i);
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = indexCount * sizeof(WORD);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices.get();
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Create the constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantShapeBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Load Textures
	hr = Texture::Create("assets/Cannon_AlbedoTransparency.png", &m_pTextureRV);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cannon_Normal.png", &m_pNormalMap);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cannon_MetallicSmoothness.png", &m_pMetallicSmoothMap);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cannon_Occlusion.png", &m_pOcclusionMap);
	if (FAILED(hr))
		return hr;

	// Create the common sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// Create the shadow map sample state
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &m_pSamplerShadow);
	if (FAILED(hr))
		return hr;

	m_IndexCount = m_MeshNode.indexArray.size();

	return hr;
}

void Model::ApplyRenderState()
{
	auto textureRV = m_pTextureRV->GetShaderResoure();
	auto normalMap = m_pNormalMap->GetShaderResoure();
	auto metallicSmoothMap = m_pMetallicSmoothMap->GetShaderResoure();
	auto occlusionMap = m_pOcclusionMap->GetShaderResoure();
	RENDER_CONTEXT::SetPixelShaderResource(0, textureRV);
	RENDER_CONTEXT::SetPixelShaderResource(5, metallicSmoothMap);
	RENDER_CONTEXT::SetPixelShaderResource(6, occlusionMap);
	RENDER_CONTEXT::SetPixelShaderResource(7, normalMap);

	RENDER_CONTEXT::SetPixelSampler(0, m_pSamplerLinear);
	RENDER_CONTEXT::SetPixelSampler(1, m_pSamplerShadow);
}

void Model::Draw()
{
	// Set index buffer
	RENDER_CONTEXT::GetImmediateContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	RENDER_CONTEXT::DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_IndexCount, 0);
}

void Model::Release()
{
	delete this;
}