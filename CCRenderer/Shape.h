#pragma once

#include <d3d11_1.h>
#include <xnamath.h>
#include "Node.h"

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