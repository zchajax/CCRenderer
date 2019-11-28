#pragma once
#include <fbxsdk.h>
#include <d3d11_1.h>
#include <vector>
#include <map>
#include <string>

struct FBX_MESH_NODE
{
	std::vector<unsigned int>		indexArray;
	std::vector<FbxVector4>			m_positionArray;
	std::vector<FbxVector4>			m_normalArray;
	std::vector<FbxVector4>			m_tangentArray;
	std::vector<FbxVector4>			m_binormaArray;
	std::vector<FbxVector2>			m_texcoordArray;
};

class FBXLoader
{
public:

	FBXLoader();

	~FBXLoader();

	HRESULT LoadFBX(const char* filename, FBX_MESH_NODE* meshNode);

	static FBXLoader* GetInstance();

	static void DestoryInstance();

protected:

	HRESULT Init();

	void TriangulateRecursive(FbxNode* pNode);

	void ProcessNode(FbxNode* node, FBX_MESH_NODE* meshNode);

	void ProcessMesh(FbxMesh* node, FBX_MESH_NODE* meshNode);

private:

	static FBXLoader* m_Instance;

	FbxManager*		m_SdkManager;
	FbxScene*		m_Scene;
	FbxImporter*	m_Impoter;
	std::map<std::string, FBX_MESH_NODE> m_MeshNodes;
};