#include "Scene.h"
#include "Node.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Cube.h"
#include "Sphere.h"
#include "Plane.h"
#include "Model.h"
#include "RenderContext.h"
#include "Rendering_Pipeline.h"

Scene::Scene()
{

}

Scene::~Scene()
{
	for (std::vector<Node*>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++)
	{
		delete* iter;
	}

	m_Nodes.clear();
}

void Scene::Init()
{
	m_MainLight = DirectionalLight::Create(
		XMFLOAT4(1.0, 1.0, 1.0, 1.0),
		XMFLOAT4(-10.0f, 10.0f, -10.0f, 1.0f),
		XMFLOAT4(-10.0f, 10.0f, -10.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		0.3, 50.0f, true);

	auto pointLight = PointLight::Create(
		XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0, 0.0, 1.0, 1.0),
		0.8);
	m_AdditionalLights.push_back(pointLight);

	auto pointLight1 = PointLight::Create(
		XMFLOAT4(3.0f, 5.0f, 3.0f, 1.0f),
		XMFLOAT4(1.0, 0.0, 0.0, 1.0),
		0.8);
	m_AdditionalLights.push_back(pointLight1);

	auto pointLight2 = PointLight::Create(
		XMFLOAT4(-3.0f, 5.0f, -3.0f, 1.0f),
		XMFLOAT4(0.0, 1.0, 0.0, 1.0),
		0.8);
	m_AdditionalLights.push_back(pointLight2);

	// Initialize skybox
	m_SkyBox.Init(RENDER_CONTEXT::GetDevice());

	Cube* cube1 = Cube::Create("assets/tes12t.dds");
	cube1->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	AddNode(cube1);

	Cube* cube2 = Cube::Create("assets/test.dds");
	cube2->SetPosition(XMFLOAT3(3.5f, 0.0f, 0.0f));
	AddNode(cube2);

	Cube* cube3 = Cube::Create("assets/test.dds");
	cube3->SetPosition(XMFLOAT3(16.0f, 0.0f, 10.0f));
	AddNode(cube3);

	Sphere* sphere1 = Sphere::Create("");
	sphere1->SetPosition(XMFLOAT3(1.0f, 0.0f, 3.0f));
	AddNode(sphere1);

	Plane* plane = Plane::Create("assets/test.dds");
	plane->SetPosition(XMFLOAT3(3.0f, -1.0f, 3.0f));
	plane->setScale(XMFLOAT3(20.0f, 1.0f, 20.0f));
	AddNode(plane);

	Model* model1 = Model::Create("assets/Cerberus_LP.fbx");
	model1->SetPosition(XMFLOAT3(2.0f, 1.0f, 5.0f));
	model1->setRotation(XMFLOAT3(-3.14f / 2, 0.0f, 0.0f));
	model1->setScale(XMFLOAT3(0.03f, 0.03f, 0.03f));
	AddNode(model1);
}

void Scene::Update(float delta)
{
	// Update skybox
	m_SkyBox.Update(delta);

	m_MainLight->Update(delta);

	for (auto light : m_AdditionalLights)
	{
		light->Update(delta);
	}

	for (auto node : m_Nodes)
	{
		node->Update(delta);
	}
}

void Scene::RenderShadowMap()
{
	// Render To ShadowMap
	if (m_MainLight->IsEnableShadow())
	{
		m_MainLight->RenderToShadowMap(m_Nodes);
	}
}

void Scene::Render()
{
	// Init Frame
	/*RENDER_CONTEXT::SetPixelShaderResource(2, m_SkyBox.GetIrradianceMap());
	RENDER_CONTEXT::SetPixelShaderResource(3, m_SkyBox.GetFilterMipmap());
	RENDER_CONTEXT::SetPixelShaderResource(4, m_SkyBox.GetBrdfMap());*/

	// Render opaque objects
	RENDER_CONTEXT::PushMarker(MARK("Render Opaque"));
	for (std::vector<Node*>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++)
	{
		if ((*iter)->GetType() == Node::NODE_TYPE_OPAQUE)
		{
			// Main Lighting
			m_MainLight->Apply();

			(*iter)->SetRenderLightType(Light::LIGHT_TYPE::DIRECTIONAL);
			(*iter)->SetEnableBlend(false);
			(*iter)->Render();

			// Additional lighting
			for (auto light : m_AdditionalLights)
			{
				light->Apply();

				(*iter)->SetRenderLightType(Light::LIGHT_TYPE::POINT);
				(*iter)->SetEnableBlend(true);
				(*iter)->Render();
			}
		}
	}
	RENDER_CONTEXT::PopMarker();

	// Render skybox
	RENDER_CONTEXT::PushMarker(MARK("Render SkyBox"));
	m_SkyBox.Render();
	RENDER_CONTEXT::PopMarker();

	// Render transparent objects
	RENDER_CONTEXT::PushMarker(MARK("Render Transparent"));
	for (std::vector<Node*>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++)
	{
		if ((*iter)->GetType() == Node::NODE_TYPE_TRANSPARENT)
		{
			(*iter)->Render();
		}
	}
	RENDER_CONTEXT::PopMarker();
}

void Scene::AddNode(Node* node, Node::NODE_TYPE type)
{
	if (node)
	{
		node->SetType(type);
		m_Nodes.push_back(node);
	}
}

void Scene::RemoveNode(Node* node)
{
	for (std::vector<Node*>::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++)
	{
		if (node == (*iter))
		{
			m_Nodes.erase(iter);

			break;
		}
	}
}