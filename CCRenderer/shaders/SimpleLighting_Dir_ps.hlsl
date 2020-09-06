#define DIRECTIONAL

#include "Common.hlsli"
#include "Shadow.hlsli"
#include "Light.hlsli"

Texture2D txDiffuse : register(t1);
SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Norm : COLOR0;
	float2 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
	float4 ClipPos : TEXCOORD2;
};

// Pixel Shaderd
void main(
	VS_OUTPUT input,
	out float4 outColor		: SV_TARGET0,
	out float4 outNormal	: SV_TARGET1,
	out float4 outDepth		: SV_TARGET2)
{
	float shadow = ComputeShadow(input.WorldPos, View);

	float4 viewDir = normalize(Eye - input.WorldPos);
	float4 h = normalize(LightDir + viewDir);
	float nh = max(0, dot(input.Norm, h));
	float spec = pow(nh, 200);

	float4 finalColor = 0;
	float4 textureColor = txDiffuse.Sample(samLinear, input.Tex);
	finalColor += vAmbientColor * textureColor;

	finalColor += max(0, dot((float4)input.Norm, LightDir)) * LightColor * Intensity * textureColor * shadow;
	finalColor += LightColor * spec * 0.8f * shadow * Intensity;

	finalColor = saturate(finalColor);

	outColor = finalColor;
	outNormal.xyz = input.Norm.xyz;
	outDepth.r = input.ClipPos.z / input.ClipPos.w;
}
