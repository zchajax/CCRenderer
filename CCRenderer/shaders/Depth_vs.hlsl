
cbuffer ConstantShapeBuffer : register(b0)
{
	matrix World;
	matrix Wvp;
}

cbuffer ConstantDepthBuffer : register(b13)
{
	matrix View;
	matrix Projection;
	float4 TextureSize;
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float Depth : TEXCOORD1;
};

// Vertex
VS_OUTPUT main( in VS_INPUT IN)
{
	VS_OUTPUT OUT;
	OUT.Pos = mul(IN.Pos, Wvp);
	OUT.Depth = OUT.Pos.z / OUT.Pos.w;
	return OUT;
}
