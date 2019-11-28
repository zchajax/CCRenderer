#include "Node.h"
#include "Texture.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "VertexBuffer.h"
#include "commonh.h"

Node::Node()
{
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
	m_pVertexLayout = NULL;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pTextureRV = NULL;
	m_pSamplerLinear = NULL;
	m_pSamplerShadow = NULL;
	m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_IndexCount = 0;
	m_IsDirty = true;
}

Node::~Node()
{
	RENDER_CONTEXT::ReleaseVertexBuffer(m_pVertexBuffer);

	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
	if (m_pTextureRV) m_pTextureRV->Release();
	if (m_pSamplerLinear) m_pSamplerLinear->Release();
	if (m_pSamplerShadow) m_pSamplerShadow->Release();
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

void Node::RenderShadowMap()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

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

	// Set vertex shader
	immediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	immediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// Set pixel shader
	immediateContext->PSSetShader(m_pPixelShader, NULL, 0);

	ApplyRenderState();

	Draw();
}