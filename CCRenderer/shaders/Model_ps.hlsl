
#define MAX_CASCADED 8

Texture2DArray shadowTexture : register(t0);
Texture2D txDiffuse : register(t1);
Texture2D metallicTexture: register(t2);
Texture2D roughnessTexture: register(t3);
Texture2D normalTexure : register(t4);

//TextureCube txCube : register(t2);
//TextureCube preFilterMap : register(t3);
//Texture2D brdfMap : register(t4);
SamplerState samLinear : register(s0);
SamplerState samShadow : register(s1);

cbuffer ConstantShadowMapBuffer : register(b1)
{
	matrix cascadedView;
	matrix cascadedProj[MAX_CASCADED];
	float4 vfCascadeFrustumsEyeSpaceDepths[2];
	float4 vTextureSize;
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

float Depth(float2 offset, float2 texUV, int level)
{
	return shadowTexture.SampleGrad(samShadow, float3(offset + texUV, float(level)), 0, 0).r;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(1.0f - roughness, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

//float3 computePBRAmbient(float3 N, float3 V, float3 albedo, float roughness, float3 metallic, float3 F0)
//{
//	float3 R = normalize(reflect(float4(-V, 0.0f), float4(N, 0.0f))).xyz;
//	float dotNV = clamp(dot(N, V), 0.0f, 1.0f);
//
//	float3 F = fresnelSchlickRoughness(dotNV, F0, roughness);
//	float3 ks = F;
//	float3 kd = 1.0f - ks;
//	kd *= 1.0f - metallic;
//
//	float3 irradiance = txCube.Sample(samLinear, N).xyz;
//	float3 diffuse = irradiance * albedo;
//
//	float3 prefilteredColor = preFilterMap.SampleLevel(samLinear, R, roughness * 4.0f).xyz;
//	float2 envBRDF = brdfMap.Sample(samLinear, float2(dotNV, roughness)).xy;
//	float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
//	float3 ambient = (kd * diffuse + specular);
//
//	return ambient;
//}

float3 computePBRLighting(float3 N, float3 V, float3 albedo, float roughness, float3 metallic, out float3 ambient)
{
	float alpha = roughness * roughness;
	float3 L = LightDir.xyz;
	float3 H = normalize(L + V).xyz;
	float dotNL = clamp(dot(N, L), 0.0f, 1.0f);
	float dotNV = clamp(dot(N, V), 0.0f, 1.0f);
	float dotNH = clamp(dot(N, H), 0.0f, 1.0f);
	float dotVH = clamp(dot(V, H), 0.0f, 1.0f);

	float D, G;
	float3 F;

	float alphaSqr = alpha * alpha;
	float pi = 3.1415926535f;
	float denom = dotNH * dotNH * (alphaSqr - 1.0f) + 1.0f;
	D = alphaSqr / (pi * denom * denom);

	float3 F0 = lerp(0.04f, albedo, metallic);
	F = fresnelSchlick(dotNL, F0);

	float k = (alphaSqr + 1.0f) * (alphaSqr + 1.0f) / 8;
	G = dotNV / (dotNV * (1.0f - k) + k) * dotNL / (dotNL * (1.0f - k) + k);

	float3 ks = F;
	float3 kd = 1.0f - ks;
	kd *= 1.0f - metallic;

	float3 numberator = D * F * G;
	float  denominator = (4 * dotNL * dotNV);
	float3 specular = numberator / max(denominator, 0.001f);
	float invPi = 0.31830988618f;

	//out ambient
	//ambient = computePBRAmbient(N, V, albedo, roughness, metallic, F0);
	ambient = float3(0.3, 0.3, 0.3);

	float3 directPbr = (kd * albedo * invPi + specular) * LightColor.xyz * dotNL;
	/*directPbr = directPbr / (directPbr + 1.0f);
	directPbr = pow(directPbr, 1.0f / 2.2f);*/
	return directPbr;
}
 
// Pixel Shaderd
void main(
	VS_OUTPUT input,
	out float4 outColor : SV_TARGET0,
	out float4 outNormal : SV_TARGET1,
	out float4 outDepth : SV_TARGET2)
{
	// compute lightview positon and cascaded level
	float4 cameraPos = mul(input.WorldPos, View);
	int level = 0;
	int counter = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			counter++;

			if (cameraPos.z > vfCascadeFrustumsEyeSpaceDepths[i][j])
			{
				level = counter;
			}
		}
	}
	level = min(level, MAX_CASCADED - 1);
	float4 LightPos = mul(input.WorldPos, cascadedView);
	LightPos = mul(LightPos, cascadedProj[level]);

	// Compute pixel's color
	float4 finalColor = 0;
	float4 textureColor = txDiffuse.Sample(samLinear, input.Tex);
	//float4 occlusion = occlusionTexture.Sample(samLinear, input.Tex);

	float2 projTexCoord;
	projTexCoord.x = LightPos.x / LightPos.w / 2.0f + 0.5f;
	projTexCoord.y = LightPos.y / LightPos.w / 2.0f * -1.0f + 0.5f;


	float4 BumpNormal = normalTexure.Sample(samLinear, input.Tex);
	BumpNormal = float4(BumpNormal.xyz * 2.0f - 1.0f, 0.0f);
	//BumpNormal.xyz = mul(BumpNormal.xyz, float3x3(input.Tangent, input.BiNorm, input.Norm));
	BumpNormal.xyz = input.Norm;
	BumpNormal.w = 0.0f;
	BumpNormal = normalize(BumpNormal);

	if ((saturate(projTexCoord.x) == projTexCoord.x) && (saturate(projTexCoord.y) == projTexCoord.y))
	{
		float scaleWidth = 1.0f / vTextureSize.x;
		float scaleHeight = 1.0f / vTextureSize.y;

		float lightDepthValue = LightPos.z / LightPos.w - 0.005f;

		float sum = 0;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				sum += Depth(float2((-1.5f + i) * scaleWidth, (-1.5f + j) * scaleHeight), projTexCoord, level) > lightDepthValue ? 1 : 0;
			}
		}
		float visibility = sum / 16;

		float4 viewDir = normalize(Eye - input.WorldPos);
		float4 metallic_smooth = metallicTexture.Sample(samLinear, input.Tex);
		float3 metallic = metallic_smooth.xyz;
		float roughness = roughnessTexture.Sample(samLinear, input.Tex);
		float3 ambient;
		finalColor.xyz += computePBRLighting(BumpNormal.xyz, viewDir.xyz, textureColor.xyz, roughness, metallic, ambient);
		finalColor *= visibility;
		//finalColor += 0.03f * textureColor * occlusion.x;

		finalColor.xyz += ambient * /*occlusion.x **/ textureColor.xyz;
		finalColor.a = 1.0f;
	}

	outColor = finalColor;
	outNormal.xyz = input.Norm;
	outNormal.w = 1.0;
	outDepth.r = input.ClipPos.z / input.ClipPos.w;
}