
cbuffer ConstantShapeBuffer : register(b0)
{
	matrix World;
	matrix Wvp;
}

cbuffer ConstantCameraBuffer : register(b1)
{
	matrix View;
	matrix Projection;
	float4 Eye;
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

// Vertex
VS_OUTPUT main( VS_INPUT input )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(input.Pos, Wvp);
	output.Tex = input.Tex;

    return output;
}
