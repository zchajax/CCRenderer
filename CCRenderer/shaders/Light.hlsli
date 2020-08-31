
#ifdef DIRECTIONAL
cbuffer ConstDirLightBuffer : register(b12)
{
	float4 vAmbientColor;
	float4 LightDir;
	float4 LightColor;
	matrix LightView;
	matrix LightProj;
}
#endif