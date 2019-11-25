#pragma once

#include "Shape.h"
class Plane : public Shape
{
public:

	static Plane* Create(const char* imagePath);

	virtual ~Plane();

protected:

	Plane();

	virtual HRESULT CreateVertexIndicesBuffer() override;
};
