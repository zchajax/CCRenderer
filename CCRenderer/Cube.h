#pragma once

#include "Shape.h"
class Cube : public Shape
{
public:

	static Cube* Create(const char* imagePath);

	virtual ~Cube();

protected:

	Cube();

	virtual HRESULT CreateVertexIndicesBuffer() override;
};