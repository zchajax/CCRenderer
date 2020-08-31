#ifndef LIGHT
#define LIGHT

#ifdef DIRECTIONAL
cbuffer ConstDirLightBuffer : register(b12)
{
	float4 vAmbientColor;
	float4 LightDir;
	float4 LightColor;
	matrix LightView;
	matrix LightProj;
	float  Intensity;
}
#endif // DIRECTIONAL

#ifdef POINT
cbuffer ConstPointLightBuffer : register(b13)
{
	float4 LightPos;
	float4 LightColor;
	float  Intensity;
}
#endif

#endif // LIGHT