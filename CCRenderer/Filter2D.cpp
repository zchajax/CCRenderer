#include "Filter2D.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Texture_DX11.h"
#include "VertexBuffer.h"
#include "commonh.h"

FILTER2D::FILTER2D(WCHAR* pixelShaderPath)
{
	// Create the vertex shader
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	RENDER_CONTEXT::LoadShader(L"shaders/Filter2D_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(pixelShaderPath, &m_pPixelShader);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	RENDER_CONTEXT::GetDevice()->CreateInputLayout(layout, numElements, &compiledShader[0],
		size, &m_pVertexLayout);

	static Vertex_PosUV g_geomQuad[6] = {
		{ XMFLOAT3{ -1.0f,  1.0f, 1.0f } , XMFLOAT2{0.0f, 0.0f} },
		{ XMFLOAT3{ 1.0f,  -1.0f, 1.0f }, XMFLOAT2{1.0f, 1.0f } },
		{ XMFLOAT3{ -1.0f, -1.0f, 1.0f }, XMFLOAT2{0.0f, 1.0f } },

		{ XMFLOAT3{ 1.0f,  1.0f, 1.0f } , XMFLOAT2{ 1.0f, 0.0f } },
		{ XMFLOAT3{ 1.0f,  -1.0f, 1.0f }, XMFLOAT2{ 1.0f, 1.0f } },
		{ XMFLOAT3{ -1.0f, 1.0f, 1.0f }, XMFLOAT2{ 0.0f, 0.0f } },
	};

	m_pVertexBuffer = RENDER_CONTEXT::CreateVertexBuffer(6, sizeof(Vertex_PosUV), g_geomQuad);
}

FILTER2D::~FILTER2D()
{
	RENDER_CONTEXT::ReleaseVertexBuffer(m_pVertexBuffer);

	if (m_pVertexShader)
	{
		m_pVertexShader->Release();

		m_pVertexShader = NULL;
	}

	if (m_pPixelShader)
	{
		m_pPixelShader->Release();

		m_pPixelShader = NULL;
	}

	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();

		m_pVertexLayout = NULL;
	}
}

void FILTER2D::Init(TEXTURE_DX11* source, RENDER_TARGET* target)
{
	RENDER_CONTEXT::SetCurrentRenderTarget(target, RCBI_COLOR_BUFFER);
	RENDER_CONTEXT::SetCurrentRenderTarget(NULL, RCBI_NORMAL_BUFFER);
	RENDER_CONTEXT::SetCurrentRenderTarget(NULL, RCBI_DEPTH_BUFFER);
	RENDER_CONTEXT::SetCurrentDepthTarget(NULL);
	RENDER_CONTEXT::ApplyRenderTargets();

	RENDER_CONTEXT::SetPixelShaderResource(0, source->GetShaderResource());
	RENDER_CONTEXT::SetPixelSampler(0, source->GetSamplerState());
}

void FILTER2D::Apply()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	// Set the input layout
	immediateContext->IASetInputLayout(m_pVertexLayout);

	// Set vertex buffer
	ID3D11Buffer* vertexBuffer = m_pVertexBuffer->GetD3DBuffer();
	UINT stride = m_pVertexBuffer->GetVertexStride();
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set vertex shader
	immediateContext->VSSetShader(m_pVertexShader, NULL, 0);

	// Set pixel shader
	immediateContext->PSSetShader(m_pPixelShader, NULL, 0);

	RENDER_CONTEXT::DrawPrimitive(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 6, 0);
}