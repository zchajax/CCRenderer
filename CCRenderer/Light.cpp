#include "Light.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "commonh.h"

Light::Light():
	m_Position(0, 0, 0, 0), 
	m_Color(0, 0, 0, 0),
	m_Intensity(0), 
	m_Type(LIGHT_TYPE::DIRECTIONAL),
	m_pConstantBuffer(NULL)
{

}

Light::~Light()
{

}

void Light::Update(float delta)
{

}