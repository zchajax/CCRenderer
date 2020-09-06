#ifndef COMMON
#define COMMON

cbuffer ConstantCameraBuffer : register(b11)
{
	matrix View;
	matrix Projection;
	float4 Eye;
}

cbuffer ConstantMaterialBuffer : register(b10)
{
	float4 Color;
}

#endif