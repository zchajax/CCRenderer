#pragma once

#include <d3d11_1.h>

class VERTEX_BUFFER
{
private:

	VERTEX_BUFFER(UINT32 vertexCount, UINT32 vertexStride, void* initData);
	
	~VERTEX_BUFFER();

public:
	
	UINT32		GetVertexCount() const { return m_uiVertexCount; }
	UINT32		GetVertexStride() const { return m_uiVertexStride; }
	ID3D11Buffer*	GetD3DBuffer() const { return m_pBuffer; }

private:

	ID3D11Buffer*	m_pBuffer;
	UINT32		m_uiVertexCount;
	UINT32		m_uiVertexStride;
	void*		m_pData;

	friend class RENDER_CONTEXT;
};
