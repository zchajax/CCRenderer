#include "Scene.h"
#include "Node.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Cube.h"
#include "Sphere.h"
#include "Plane.h"
#include "Model.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "Rendering_Pipeline.h"
#include <algorithm> 

bool OpaqueSort(Node* node1, Node* node2)
{ 
	return (node1->GetDistanceFromCamera() < node2->GetDistanceFromCamera());
}

bool TransparentSort(Node* node1, Node* node2)
{
	return (node1->GetDistanceFromCamera() > node2->GetDistanceFromCamera());
}

Scene::Scene()
{

}

Scene::~Scene()
{
	for (std::vector<Node*>::iterator iter = m_OpaqueNodes.begin(); iter != m_OpaqueNodes.end(); iter++)
	{
		delete *iter;
	}
	m_OpaqueNodes.clear();

	for (std::vector<Node*>::iterator iter = m_TransparentNodes.begin(); iter != m_TransparentNodes.end(); iter++)
	{
		delete* iter;
	}
	m_TransparentNodes.clear();
}

void Scene::Init()
{
	m_MainLight = DirectionalLight::Create(
		XMFLOAT4(1.0, 1.0, 1.0, 1.0),
		XMFLOAT4(-10.0f, 10.0f, -10.0f, 1.0f),
		XMFLOAT4(-10.0f, 10.0f, -10.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		1.0, 50.0f, true);

	auto pointLight = PointLight::Create(
		XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0, 0.0, 1.0, 1.0),
		1.0);
	m_AdditionalLights.push_back(pointLight);

	auto pointLight1 = PointLight::Create(
		XMFLOAT4(3.0f, 5.0f, 3.0f, 1.0f),
		XMFLOAT4(1.0, 0.0, 0.0, 1.0),
		1.0);
	m_AdditionalLights.push_back(pointLight1);

	auto pointLight2 = PointLight::Create(
		XMFLOAT4(-3.0f, 5.0f, -3.0f, 1.0f),
		XMFLOAT4(0.0, 1.0, 0.0, 1.0),
		1.0);
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

	Sphere* sphere2 = Sphere::Create("");
	sphere2->SetPosition(XMFLOAT3(-1.0f, 0.0f, -3.0f));
	sphere2->SetColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 0.2f));
	AddNode(sphere2, Node::NODE_TYPE_TRANSPARENT);

	Sphere* sphere3 = Sphere::Create("");
	sphere3->SetPosition(XMFLOAT3(-3.0f, 0.0f, -5.0f));
	sphere3->SetColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 0.2f));
	AddNode(sphere3, Node::NODE_TYPE_TRANSPARENT);

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

	for (auto node : m_OpaqueNodes)
	{
		node->Update(delta);
	}

	for (auto node : m_TransparentNodes)
	{
		node->Update(delta);
	}

	SortOpaques();

	SortTransparents();
}

void Scene::RenderShadowMap()
{
	// Render To ShadowMap
	if (m_MainLight->IsEnableShadow())
	{
		m_MainLight->RenderToShadowMap(m_OpaqueNodes);
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
	for (std::vector<Node*>::iterator iter = m_OpaqueNodes.begin(); iter != m_OpaqueNodes.end(); iter++)
	{
		(*iter)->EnableDepthWrite();

		// Main Lighting
		m_MainLight->Apply();
		(*iter)->SetRenderLightType(Light::LIGHT_TYPE::DIRECTIONAL);
		(*iter)->Render();

		// Additional lighting
		for (auto light : m_AdditionalLights)
		{
			light->Apply();

			(*iter)->SetRenderLightType(Light::LIGHT_TYPE::POINT);
			(*iter)->SetBlendAdd();
			(*iter)->Render();
		}
	}
	RENDER_CONTEXT::PopMarker();

	// Render skybox
	RENDER_CONTEXT::PushMarker(MARK("Render SkyBox"));
	m_SkyBox.Render();
	RENDER_CONTEXT::PopMarker();

	// Render transparent objects
	RENDER_CONTEXT::PushMarker(MARK("Render Transparent"));
	for (std::vector<Node*>::iterator iter = m_TransparentNodes.begin(); iter != m_TransparentNodes.end(); iter++)
	{
		(*iter)->DisableDepthWrite();

		// Main Lighting
		m_MainLight->Apply();

		(*iter)->SetRenderLightType(Light::LIGHT_TYPE::DIRECTIONAL);
		(*iter)->SetBlendTransparent();
		(*iter)->Render();

		// Additional lighting
		for (auto light : m_AdditionalLights)
		{
			light->Apply();

			(*iter)->SetRenderLightType(Light::LIGHT_TYPE::POINT);
			(*iter)->SetBlendAdd();
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

		if (type == Node::NODE_TYPE_OPAQUE)
		{
			m_OpaqueNodes.push_back(node);
		}
		else
		{
			m_TransparentNodes.push_back(node);
		}
	}
}

void Scene::RemoveNode(Node* node)
{
	std::vector<Node*> temp;

	if (node->GetType() == Node::NODE_TYPE_OPAQUE)
	{
		temp = m_OpaqueNodes;
	}
	else
	{
		temp = m_TransparentNodes;
	}

	for (std::vector<Node*>::iterator iter = temp.begin(); iter != temp.end(); iter++)
	{
		if (node == (*iter))
		{
			temp.erase(iter);

			break;
		}
	}
}

void Scene::SortOpaques()
{
	auto& camera = GameApp::getInstance()->getMainCamera();

	for (auto opaque : m_OpaqueNodes)
	{
		auto worldPos = XMLoadFloat3(&opaque->GetPosition());
		auto viewMatrix = XMLoadFloat4x4(&camera.GetViewMatrix());
		auto projMatrix = XMLoadFloat4x4(&camera.GetProjMatrix());
		auto projPos = XMVector3Transform(worldPos, viewMatrix * projMatrix);

		opaque->SetDistanceFromCamera(projPos.m128_f32[2]);
	}

	std::sort(m_OpaqueNodes.begin(), m_OpaqueNodes.end(), OpaqueSort);
}

void Scene::SortTransparents()
{
	auto& camera = GameApp::getInstance()->getMainCamera();

	for (auto tranparent : m_TransparentNodes)
	{
		auto worldPos = XMLoadFloat3(&tranparent->GetPosition());
		auto viewMatrix = XMLoadFloat4x4(&camera.GetViewMatrix());
		auto projMatrix = XMLoadFloat4x4(&camera.GetProjMatrix());
		auto projPos = XMVector3Transform(worldPos, viewMatrix * projMatrix);

		tranparent->SetDistanceFromCamera(projPos.m128_f32[2]);

		std::sort(m_TransparentNodes.begin(), m_TransparentNodes.end(), TransparentSort);
	}
}