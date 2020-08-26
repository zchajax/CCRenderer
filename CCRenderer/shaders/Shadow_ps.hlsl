
#define MAX_CASCADED 8

Texture2DArray shadowTexture : register(t0);
Texture2D txDiffuse : register(t1);

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
}

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Norm : COLOR0;
	float2 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
	float4 ClipPos : TEXCOORD2;
};

float Depth(float2 offset, float2 texUV, int level)
{
	//return shadowTexture.Sample(samShadow, offset + texUV).r;

	return shadowTexture.SampleGrad(samShadow, float3(offset + texUV, level), 0, 0).r;
}

// Pixel Shaderd
void main(
	VS_OUTPUT input,
	out float4 outColor		: SV_TARGET0,
	out float4 outNormal	: SV_TARGET1,
	out float4 outDepth		: SV_TARGET2)
{
	// compute lightview positon and cascaded level
	float4 cameraPos = mul(input.WorldPos, View);
	int level = 0;
	int counter = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			counter = counter + 1;

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
	finalColor += vAmbientColor * textureColor;

	float2 projTexCoord;

	projTexCoord.x = LightPos.x / LightPos.w / 2.0f + 0.5f;
	projTexCoord.y = LightPos.y / LightPos.w / -2.0f + 0.5f;

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
		float4 h = normalize(LightDir + viewDir);
		float nh = max(0, dot(input.Norm, h));
		float spec = pow(nh, 200);

		finalColor += max(0, dot((float4)input.Norm, LightDir)) * LightColor * textureColor * visibility;
		finalColor += LightColor * spec * 0.8f * visibility;

		finalColor = saturate(finalColor);
	}

	outColor = finalColor;
	outNormal.xyz = input.Norm.xyz;
	outDepth.r = input.ClipPos.z / input.ClipPos.w;
}
