cbuffer ConstantShapeBuffer : register(b0)
{
	matrix World;
	matrix Wvp;
}

cbuffer ConstantCameraBuffer : register(b11)
{
	matrix View;
	matrix Projection;
	float4 Eye;
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
	float4 Norm : COLOR0;
	float2 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
	float4 ClipPos : TEXCOORD2;
};

// Vertex
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Pos = mul(input.Pos, Wvp);
	output.ClipPos = output.Pos;

	output.Norm = normalize(mul(float4(input.Norm, 0.0f), World));
	output.Tex = input.Tex;
	output.WorldPos = mul(input.Pos, World);

	return (output);
}
