#include "Shape.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Camera.h"
#include "commonh.h"

Shape::Shape()
{

}

Shape::~Shape()
{

}

HRESULT Shape::Init(const char* imagePath)
{
	HRESULT hr = S_OK;

	GameApp* app = GameApp::getInstance();

	// Create the vertex shader
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	RENDER_CONTEXT::LoadShader(L"shaders/Shadow_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(L"shaders/Shadow_ps.cso", &m_pPixelShader);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = RENDER_CONTEXT::GetDevice()->CreateInputLayout(layout, numElements, &compiledShader[0],
		size, &m_pVertexLayout);
	if (FAILED(hr))
		return hr;

	// Create vertex buffer and indices buffer
	hr = CreateVertexIndicesBuffer();
	if (FAILED(hr))
		return hr;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantShapeBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = Texture::Create(imagePath, &m_pTextureRV);
	if (FAILED(hr))
		return hr;

	// Create the albedo texture sample state
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

	return S_OK;
}

void Shape::ApplyRenderState()
{
	auto textureRV = m_pTextureRV->GetShaderResoure();
	RENDER_CONTEXT::SetPixelShaderResource(0, textureRV);

	RENDER_CONTEXT::SetPixelSampler(0, m_pSamplerLinear);
	RENDER_CONTEXT::SetPixelSampler(1, m_pSamplerShadow);
}

void Shape::Draw()
{
	// Set index buffer
	RENDER_CONTEXT::GetImmediateContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	RENDER_CONTEXT::DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_IndexCount, 0);
}