#pragma once

#include "FBXLoader.h"
#include "Node.h"
#include <DirectXMath.h>
using namespace DirectX;

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
	Texture*					m_pMetallicMap;
	Texture*					m_pRoughnessMap;
	FBX_MESH_NODE				m_MeshNode;
};