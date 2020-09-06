#include "Shape.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Camera.h"
#include "SamplerStateManager.h"
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
	RENDER_CONTEXT::LoadShader(L"shaders/SimpleLighting_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(L"shaders/SimpleLighting_Point_ps.cso", &m_pPixelShaderPoint);
	RENDER_CONTEXT::CreatePixelShader(L"shaders/SimpleLighting_Dir_ps.cso", &m_pPixelShaderDirecional);

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

	// Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC disableDepthDesc;
	ZeroMemory(&disableDepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	disableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	disableDepthDesc.DepthEnable = true;
	disableDepthDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	RENDER_CONTEXT::GetDevice()->CreateDepthStencilState(&disableDepthDesc, &m_pStencilState);

	// Create Blend state
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = ((D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN) | D3D11_COLOR_WRITE_ENABLE_BLUE);
	RENDER_CONTEXT::GetDevice()->CreateBlendState(&blendDesc, &m_BlendState);

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
	m_pSamplerLinear = SamplerStateManager::GetInstance()->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_NEVER);

	// Create the shadow map sample state
	m_pSamplerShadow = SamplerStateManager::GetInstance()->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_ALWAYS);

	return S_OK;
}

void Shape::ApplyRenderState()
{
	auto textureRV = m_pTextureRV->GetShaderResoure();
	RENDER_CONTEXT::SetPixelShaderResource(1, textureRV);

	RENDER_CONTEXT::SetPixelSampler(0, m_pSamplerLinear->GetSamplerState());
	RENDER_CONTEXT::SetPixelSampler(1, m_pSamplerShadow->GetSamplerState());

	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();
	immediateContext->OMSetDepthStencilState(m_pStencilState, NULL);
}

void Shape::Draw()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (m_IsEnableBlend)
	{
		immediateContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);
	}
	else
	{
		immediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	}

	// Set index buffer
	RENDER_CONTEXT::GetImmediateContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	RENDER_CONTEXT::DrawIndexedPrimitive(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_IndexCount, 0);

	immediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
}