cbuffer ConstantCameraBuffer : register(b11)
{
	matrix View;
	matrix Projection;
	float4 Eye;
}