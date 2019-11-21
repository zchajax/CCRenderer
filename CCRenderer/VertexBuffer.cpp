#include "VertexBuffer.h"
#include "RenderContext.h"
#include <assert.h>

VERTEX_BUFFER::VERTEX_BUFFER(UINT32 vertexCount, UINT32 vertexStride, void* initData) :
	m_pBuffer(nullptr),
	m_uiVertexCount(vertexCount),
	m_uiVertexStride(vertexStride),
	m_pData(initData)
{
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.ByteWidth = m_uiVertexCount * m_uiVertexStride;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitialData;
	ZeroMemory(&InitialData, sizeof(InitialData));
	InitialData.pSysMem = initData;

	HRESULT hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&BufferDesc, &InitialData, &m_pBuffer);
	assert(SUCCEEDED(hr));
}

VERTEX_BUFFER::~VERTEX_BUFFER()
{
	if (m_pBuffer)
	{
		m_pBuffer->Release();

		m_pBuffer = nullptr;
	}
}
