
#define DIRECTIONAL

#include "Common.hlsli"
#include "Lighting.hlsli"
#include "PBRInput.hlsli"
#include "Shadow.hlsli"
#include "Light.hlsli"

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : COLOR0;
	float3 Tangent : COLOR1;
	float3 BiNorm : COLOR2;
	float2 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
	float4 ClipPos : TEXCOORD2;
};
 
// Pixel Shaderd
void main(
	VS_OUTPUT input,
	out float4 outColor : SV_TARGET0,
	out float4 outNormal : SV_TARGET1,
	out float4 outDepth : SV_TARGET2)
{
	float4 finalColor = 0;

	float shadow = ComputeShadow(input.WorldPos, View);

	float4 textureColor = txDiffuse.Sample(samLinear, input.Tex);
	//float4 occlusion = occlusionTexture.Sample(samLinear, input.Tex);

	float4 BumpNormal = normalTexure.Sample(samLinear, input.Tex);
	BumpNormal = float4(BumpNormal.xyz * 2.0f - 1.0f, 0.0f);
	//BumpNormal.xyz = mul(BumpNormal.xyz, float3x3(input.Tangent, input.BiNorm, input.Norm));
	BumpNormal.xyz = input.Norm;
	BumpNormal.w = 0.0f;
	BumpNormal = normalize(BumpNormal);

	float4 viewDir = normalize(Eye - input.WorldPos);
	float4 metallic_smooth = metallicTexture.Sample(samLinear, input.Tex);
	float metallic = metallic_smooth.r;
	float roughness = roughnessTexture.Sample(samLinear, input.Tex);
	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	finalColor.xyz += computePBRLighting(LightDir.xyz, BumpNormal.xyz, viewDir.xyz, LightColor.rgb, textureColor.xyz, Intensity, roughness, metallic, ambient);
	finalColor *= shadow;
	//finalColor += 0.03f * textureColor * occlusion.x;

	//finalColor.xyz += ambient * /*occlusion.x **/ textureColor.xyz;
	finalColor.a = 1.0f;

	outColor = finalColor;
	outNormal.xyz = input.Norm;
	outNormal.w = 1.0;
	outDepth.r = input.ClipPos.z / input.ClipPos.w;
}