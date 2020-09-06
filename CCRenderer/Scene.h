#pragma once
#include <vector>
#include "Node.h"
#include "SkyBox.h"

class DirectionalLight;
class Light;
class Scene
{
public:

	Scene();

	~Scene();

	void Init();

	void Update(float delta);

	void RenderShadowMap();

	void Render();

	void AddNode(Node* node, Node::NODE_TYPE type = Node::NODE_TYPE_OPAQUE);

	void RemoveNode(Node* node);

	DirectionalLight* GetMainLight()
	{
		return m_MainLight;
	}

private:

	void SortOpaques();

	void SortTransparents();

private:


	std::vector<Node*>	m_OpaqueNodes;
	std::vector<Node*>	m_TransparentNodes;

	DirectionalLight*	m_MainLight;
	std::vector<Light*> m_AdditionalLights;

	SkyBox				m_SkyBox;
};