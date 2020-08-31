#ifndef BRDF
#define BRDF

#include "Light.hlsli"

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(1.0f - roughness, F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 computePBRLighting(float3 L, float3 N, float3 V, float3 lightColor, float3 albedo, float intensity, float roughness, float metallic, out float3 ambient)
{
	lightColor *= intensity;

	float alpha = roughness * roughness;
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
	/*float3 kd = 1.0f - ks;
	kd *= 1.0f - metallic;*/
	float3 kd = 1.0f - metallic;

	float3 numberator = D * F * G;
	float  denominator = (4 * dotNL * dotNV);
	float3 specular = numberator / max(denominator, 0.001f);
	float invPi = 0.31830988618f;

	//out ambient
	//ambient = computePBRAmbient(N, V, albedo, roughness, metallic, F0);
#ifdef DIRECTIONAL
	ambient = vAmbientColor;
#else
	ambient = float3(0, 0, 0);
#endif

	float3 directPbr = (kd * albedo * invPi + specular) * lightColor.xyz * dotNL;
	/*directPbr = directPbr / (directPbr + 1.0f);
	directPbr = pow(directPbr, 1.0f / 2.2f);*/
	return directPbr;
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

#endif