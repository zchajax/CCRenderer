#pragma once

#include <d3d11_1.h>
#include "Node.h"
#include <DirectXMath.h>
using namespace DirectX;

class Shape : public Node
{
public:

	virtual ~Shape();

	virtual void ApplyRenderState() override;

	virtual void Draw() override;

protected:

	Shape();

	HRESULT Init(const char* imagePath);

	// Create vertex and indices buffer
	virtual HRESULT CreateVertexIndicesBuffer() = 0;
};