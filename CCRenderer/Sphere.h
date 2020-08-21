#pragma once

#include "Shape.h"
class Sphere : public Shape
{
public:

	static Sphere* Create(const char* imagePath);

	virtual ~Sphere();

protected:

	Sphere();

	virtual HRESULT CreateVertexIndicesBuffer() override;
};