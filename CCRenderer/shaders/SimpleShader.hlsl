
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantShapeBuffer : register(b0)
{
	matrix World;
}

cbuffer ConstantCameraBuffer : register(b11)
{
	matrix View;
	matrix Projection;
	float4 Eye;
}

cbuffer ConstLightBuffer : register(b12)
{
	float4 vAmbientColor;
	float4 LightDir;
	float4 LightColor;
	matrix LightView;
	matrix LightProj;
	float Shininess;
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
	float4 WorldPos : WORLD_POSITION;
};

// Pixel Shader
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float4 viewDir = normalize(Eye - input.WorldPos);
	float4 h = normalize(LightDir + viewDir);
	float nh = max(0, dot(input.Norm, h));
	float spec = pow(nh, Shininess * 2000);

	float4 finalColor = 0;
	float4 mainColor = txDiffuse.Sample(samLinear, input.Tex);
	finalColor += saturate(max(0, dot((float4)input.Norm, LightDir)) * LightColor * mainColor + vAmbientColor * mainColor + LightColor * spec * 0.8f);
	return finalColor;
}


void main(in VS_INPUT IN, out VS_OUTPUT OUT)
{
	OUT.Pos = mul(IN.Pos, World);
	OUT.Pos = mul(OUT.Pos, View);
	OUT.Pos = mul(OUT.Pos, Projection);
	OUT.Norm = normalize(mul(IN.Norm, World));
	OUT.Tex = IN.Tex;
	OUT.WorldPos = mul(IN.Pos, World);
}