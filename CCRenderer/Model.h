#pragma once

#include "FBXLoader.h"
#include <xnamath.h>
#include "Node.h"

class Texture;
class Model : public Node
{
public:

	static Model* Create(const char* filename);

	void Release();

	virtual void ApplyRenderState() override;

	virtual void Draw() override;

protected:

	Model();

	~Model();

	HRESULT Init(const char* filename);

private:

	Texture*					m_pNormalMap;
	Texture*					m_pMetallicSmoothMap;
	Texture*					m_pOcclusionMap;
	FBX_MESH_NODE				m_MeshNode;
};