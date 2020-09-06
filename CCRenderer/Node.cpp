#include "Node.h"
#include "Texture.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "commonh.h"

Node::Node()
{
	m_pVertexShader = NULL;
	m_pPixelShaderPoint = NULL;
	m_pPixelShaderDirecional = NULL;
	m_pVertexLayout = NULL;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pTextureRV = NULL;
	m_pSamplerLinear = NULL;
	m_pSamplerShadow = NULL;
	m_pStencilStateOpaque = NULL;
	m_pStencilStateTransparent = NULL;
	m_pStencilStateCurrent = NULL;
	m_BlendStateAdd = NULL;
	m_BlendStateTransparent = NULL;
	m_BlendStateCurrent = NULL;
	m_pConstantBuffer = NULL;
	m_pMaterialBuffer = NULL;
	m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_DistanceFromCamera = 0.0f;
	m_IndexCount = 0;
	m_IsDirty = true;
	m_RenderLightType = Light::DIRECTIONAL;
}

Node::~Node()
{
	RENDER_CONTEXT::ReleaseVertexBuffer(m_pVertexBuffer);

	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShaderPoint) m_pPixelShaderPoint->Release();
	if (m_pPixelShaderDirecional) m_pPixelShaderDirecional->Release();
	if (m_pTextureRV) m_pTextureRV->Release();
	if (m_pStencilStateOpaque) m_pStencilStateOpaque->Release();
	if (m_pStencilStateTransparent) m_pStencilStateTransparent->Release();
	if (m_BlendStateAdd) m_BlendStateAdd->Release();
	if (m_BlendStateTransparent) m_BlendStateTransparent->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pMaterialBuffer) m_pMaterialBuffer->Release();
}

HRESULT Node::InitNode()
{
	HRESULT hr;

	// Create add depth stencil state
	D3D11_DEPTH_STENCIL_DESC disableDepthDesc;
	ZeroMemory(&disableDepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	disableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	disableDepthDesc.DepthEnable = true;
	disableDepthDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	hr = RENDER_CONTEXT::GetDevice()->CreateDepthStencilState(&disableDepthDesc, &m_pStencilStateOpaque);
	if (FAILED(hr))
		return hr;

	// Create transparent depth stencil state
	disableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = RENDER_CONTEXT::GetDevice()->CreateDepthStencilState(&disableDepthDesc, &m_pStencilStateTransparent);
	if (FAILED(hr))
		return hr;

	// Create Add Blend state
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
	hr = RENDER_CONTEXT::GetDevice()->CreateBlendState(&blendDesc, &m_BlendStateAdd);
	if (FAILED(hr))
		return hr;

	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	hr = RENDER_CONTEXT::GetDevice()->CreateBlendState(&blendDesc, &m_BlendStateTransparent);
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

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantMaterialBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pMaterialBuffer);
	if (FAILED(hr))
		return hr;

}

void Node::Update(float delta)
{
	if (m_IsDirty)
	{
		auto world = XMMatrixIdentity();
		XMMATRIX translate = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
		XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		XMMATRIX rotationX = XMMatrixRotationX(m_Rotation.x);
		XMMATRIX rotationY = XMMatrixRotationY(m_Rotation.y);
		XMMATRIX rotationZ = XMMatrixRotationZ(m_Rotation.z);
		world = world * scale * rotationX * rotationY * rotationZ * translate;

		XMStoreFloat4x4(&m_World, world);

		m_IsDirty = false;
	}

	auto& camera = GameApp::getInstance()->getMainCamera();
	auto world = XMLoadFloat4x4(&m_World);
	auto view = XMLoadFloat4x4(&camera.GetViewMatrix());
	auto proj = XMLoadFloat4x4(&camera.GetProjMatrix());
	auto wvp = world * view * proj;

	XMStoreFloat4x4(&m_Wvp, wvp);
}

void Node::RenderToDepthTexture()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	// Enable depth write
	immediateContext->OMSetDepthStencilState(m_pStencilStateOpaque, NULL);

	// Update Constant Buffer
	ConstantShapeBuffer cb;
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&m_World));
	cb.mWvp = XMMatrixTranspose(XMLoadFloat4x4(&m_Wvp));
	immediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set the input layout
	immediateContext->IASetInputLayout(m_pVertexLayout);

	// Set vertex buffer
	ID3D11Buffer* vertexBuffer = m_pVertexBuffer->GetD3DBuffer();
	UINT stride = m_pVertexBuffer->GetVertexStride();
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set index buffer
	immediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	Draw();
}

void Node::Render()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	// Update Constant Buffer
	ConstantShapeBuffer cb;
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&m_World));
	cb.mWvp = XMMatrixTranspose(XMLoadFloat4x4(&m_Wvp));
	immediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	ConstantMaterialBuffer mcb;
	mcb.Color = m_Color;
	immediateContext->UpdateSubresource(m_pMaterialBuffer, 0, NULL, &mcb, 0, 0);
	// Set the input layout
	immediateContext->IASetInputLayout(m_pVertexLayout);

	// Set vertex buffer
	ID3D11Buffer* vertexBuffer = m_pVertexBuffer->GetD3DBuffer();
	UINT stride = m_pVertexBuffer->GetVertexStride();
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set vertex shader
	immediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	immediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// Set pixel shader
	if (m_RenderLightType == Light::DIRECTIONAL)
	{
		immediateContext->PSSetShader(m_pPixelShaderDirecional, NULL, 0);
		immediateContext->PSSetConstantBuffers(10, 1, &m_pMaterialBuffer);
	}
	else if (m_RenderLightType == Light::POINT)
	{
		immediateContext->PSSetShader(m_pPixelShaderPoint, NULL, 0);
	}

	ApplyRenderState();

	Draw();
}