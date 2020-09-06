#include "Model.h"
#include "Texture.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "SamplerStateManager.h"
#include "commonh.h"
#include <memory>

Model* Model::Create(const char* filename)
{
	Model* model = new Model();

	if (model)
	{
		HRESULT hr = model->Init(filename);

        if (FAILED(hr))
        {
            delete model;
            return nullptr;
        }
	}

	return model;
}

Model::Model()
{
	m_pNormalMap = NULL;
	m_pMetallicMap = NULL;
	m_pRoughnessMap = NULL;
}

Model::~Model()
{
	if (m_pNormalMap) m_pNormalMap->Release();
	if (m_pMetallicMap) m_pMetallicMap->Release();
	if (m_pRoughnessMap) m_pRoughnessMap->Release();
}

HRESULT Model::Init(const char* filename)
{
	HRESULT hr;

	hr = InitNode();
	if (FAILED(hr))
		return hr;

	// Create fbx data
	hr = FBXLoader::GetInstance()->LoadFBX(filename, &m_MeshNode);

	if (FAILED(hr))
	{
		return hr;
	}

	// Create the vertex shader
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	RENDER_CONTEXT::LoadShader(L"shaders/Lighting_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(L"shaders/Lighting_Point_ps.cso", &m_pPixelShaderPoint);
	RENDER_CONTEXT::CreatePixelShader(L"shaders/Lighting_Dir_ps.cso", &m_pPixelShaderDirecional);

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

	// Load Textures
	hr = Texture::Create("assets/Cerberus_A.dds", &m_pTextureRV);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cerberus_N.dds", &m_pNormalMap);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cerberus_M.dds", &m_pMetallicMap);
	if (FAILED(hr))
		return hr;

	hr = Texture::Create("assets/Cerberus_R.dds", &m_pRoughnessMap);
	if (FAILED(hr))
		return hr;

	// Create the albedo texture sample state
	m_pSamplerLinear = SamplerStateManager::GetInstance()->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_NEVER);

	// Create the shadow map sample state
	m_pSamplerShadow = SamplerStateManager::GetInstance()->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_ALWAYS);

	m_IndexCount = m_MeshNode.indexArray.size();

	return hr;
}

void Model::ApplyRenderState()
{
	auto textureRV = m_pTextureRV->GetShaderResoure();
	auto normalMap = m_pNormalMap->GetShaderResoure();
	auto metallicMap = m_pMetallicMap->GetShaderResoure();
	auto roughnessMap = m_pRoughnessMap->GetShaderResoure();
	RENDER_CONTEXT::SetPixelShaderResource(1, textureRV);
	RENDER_CONTEXT::SetPixelShaderResource(2, metallicMap);
	RENDER_CONTEXT::SetPixelShaderResource(3, roughnessMap);
	RENDER_CONTEXT::SetPixelShaderResource(4, normalMap);

	RENDER_CONTEXT::SetPixelSampler(0, m_pSamplerLinear->GetSamplerState());
	RENDER_CONTEXT::SetPixelSampler(1, m_pSamplerShadow->GetSamplerState());

	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	if (m_pStencilStateCurrent)
	{
		immediateContext->OMSetDepthStencilState(m_pStencilStateCurrent, NULL);
	}
}

void Model::Draw()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (m_BlendStateCurrent)
	{
		immediateContext->OMSetBlendState(m_BlendStateCurrent, blendFactor, 0xffffffff);
	}
	else
	{
		immediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	}

	// Set index buffer
	RENDER_CONTEXT::GetImmediateContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	RENDER_CONTEXT::DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_IndexCount, 0);

	immediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	m_BlendStateCurrent = NULL;
}

void Model::Release()
{
	delete this;
}