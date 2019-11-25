#include "FBXLoader.h"
#include <assert.h>

FBXLoader* FBXLoader::m_Instance = NULL;

FBXLoader* FBXLoader::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new FBXLoader();

		m_Instance->Init();
	}

	return m_Instance;
}

void FBXLoader::DestoryInstance()
{
	if (m_Instance)
	{
		delete m_Instance;
	}
}

FBXLoader::FBXLoader()
{
	m_SdkManager = NULL;

	m_Scene = NULL;

	m_Impoter = NULL;
}

FBXLoader::~FBXLoader()
{
	m_SdkManager->Destroy();
}

HRESULT FBXLoader::LoadFBX(const char* filename, FBX_MESH_NODE* meshNode)
{
	HRESULT hr = S_OK;

	std::map<std::string, FBX_MESH_NODE>::iterator iter;
	if ((iter = m_MeshNodes.find(filename)) != m_MeshNodes.end())
	{
		*meshNode = (*iter).second;

		return S_OK;
	}
	
	if (!m_Impoter || m_Impoter->Initialize(filename, -1, m_SdkManager->GetIOSettings()) == false)
		return E_FAIL;

	if (!m_Impoter || m_Impoter->Import(m_Scene) == false)
		return E_FAIL;

	FbxAxisSystem OurAxisSystem = FbxAxisSystem::DirectX;

	FbxAxisSystem SceneAxisSystem = m_Scene->GetGlobalSettings().GetAxisSystem();
	if (SceneAxisSystem != OurAxisSystem)
	{
		FbxAxisSystem::DirectX.ConvertScene(m_Scene);
	}

	FbxSystemUnit SceneSystemUnit = m_Scene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{

		FbxSystemUnit::cm.ConvertScene(m_Scene);
	}

	TriangulateRecursive(m_Scene->GetRootNode());

	ProcessNode(m_Scene->GetRootNode(), meshNode);

	m_MeshNodes[filename] = *meshNode;

	return hr;
}

HRESULT FBXLoader::Init()
{
	m_SdkManager = FbxManager::Create();

	if (!m_SdkManager)
	{
		return E_FAIL;
	}

	FbxIOSettings* ios = FbxIOSettings::Create(m_SdkManager, IOSROOT);
	m_SdkManager->SetIOSettings(ios);

	FbxString lPath = FbxGetApplicationDirectory();
	m_SdkManager->LoadPluginsDirectory(lPath.Buffer());

	m_Scene = FbxScene::Create(m_SdkManager, "");

	if (!m_Scene)
	{
		return E_FAIL;
	}

	m_Impoter = FbxImporter::Create(m_SdkManager, "");

	if (!m_Impoter)
	{
		return E_FAIL;
	}

	return S_OK;
}

void FBXLoader::TriangulateRecursive(FbxNode* pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(pNode->GetFbxManager());

			lConverter.Triangulate(m_Scene, true);
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		TriangulateRecursive(pNode->GetChild(lChildIndex));
	}
}

void FBXLoader::ProcessNode(FbxNode* node, FBX_MESH_NODE* meshNode)
{
	FbxMesh* mesh = node->GetMesh();

	if (mesh)
	{
		const int vertexCount = mesh->GetControlPointsCount();

		if (vertexCount > 0)
		{
			ProcessMesh(mesh, meshNode);
		}
	}

	const int lMaterialCount = node->GetMaterialCount();

	int count = node->GetChildCount();

	for (int i = 0; i < count; ++i)
	{
		ProcessNode(node->GetChild(i), meshNode);
	}
}

void FBXLoader::ProcessMesh(FbxMesh* mesh, FBX_MESH_NODE* meshNode)
{
	FbxVector4 pos, nor, tangent, binormal;

	int polygonCount = mesh->GetPolygonCount();

	auto pTangent = mesh->GetElementTangent(0);
	auto pNormal = mesh->GetElementNormal(0);
	auto pBinormal = mesh->GetElementBinormal(0);

	unsigned int indx = 0;

	for (int i = 0; i < polygonCount; ++i)
	{
		int lPolygonsize = mesh->GetPolygonSize(i);

		for (int j = 0; j < lPolygonsize; ++j)
		{
			int index = mesh->GetPolygonVertex(i, j);
			pos = mesh->GetControlPointAt(index);
			//mesh->GetPolygonVertexNormal(i, j, nor);
			nor = pNormal->GetDirectArray().GetAt(indx);
			tangent = pTangent->GetDirectArray().GetAt(indx);
			binormal = pBinormal->GetDirectArray().GetAt(indx);
			meshNode->indexArray.push_back(indx);
			meshNode->m_positionArray.push_back(pos);
			meshNode->m_normalArray.push_back(nor);
			meshNode->m_tangentArray.push_back(tangent);
			meshNode->m_binormaArray.push_back(binormal);
			++indx;
		}
	}

	FbxStringList uvsetName;
	mesh->GetUVSetNames(uvsetName);
	int numUVSet = uvsetName.GetCount();

	bool unmapped = false;

	for (int uv = 0; uv < numUVSet; uv++)
	{
		for (int i = 0; i < polygonCount; i++)
		{
			int lPolygonsize = mesh->GetPolygonSize(i);

			for (int pol = 0; pol < lPolygonsize; pol++)
			{
				FbxString name = uvsetName.GetStringAt(uv);
				FbxVector2 texCoord;
				mesh->GetPolygonVertexUV(i, pol, name, texCoord, unmapped);
				meshNode->m_texcoordArray.push_back(texCoord);
			}
		}
	}
}