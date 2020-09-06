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

	std::vector<Node*>& GetNodes()
	{
		return m_Nodes;
	}

	DirectionalLight* GetMainLight()
	{
		return m_MainLight;
	}

private:

	std::vector<Node*>	m_Nodes;

	DirectionalLight*	m_MainLight;
	std::vector<Light*> m_AdditionalLights;

	SkyBox				m_SkyBox;
};