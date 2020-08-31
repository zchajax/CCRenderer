#ifndef SHADOW
#define SHADOW

#define MAX_CASCADED 8

Texture2DArray shadowTexture : register(t0);
SamplerState samShadow : register(s1);

cbuffer ConstantShadowMapBuffer : register(b1)
{
	matrix cascadedView;
	matrix cascadedProj[MAX_CASCADED];
	float4 vfCascadeFrustumsEyeSpaceDepths[2];
	float4 vTextureSize;
}

float Depth(float2 offset, float2 texUV, int level)
{
	return shadowTexture.SampleGrad(samShadow, float3(offset + texUV, float(level)), 0, 0).r;
}

int ComputeCascadedLevel(float4 worldPos, matrix viewMat)
{
	float4 cameraPos = mul(worldPos, viewMat);
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

	return level;
}

float ComputeShadow(float4 worldPos, matrix viewMat)
{
	int level = ComputeCascadedLevel(worldPos, viewMat);
	float4 LightPos = mul(worldPos, cascadedView);
	LightPos = mul(LightPos, cascadedProj[level]);

	float2 projTexCoord;
	projTexCoord.x = LightPos.x / LightPos.w / 2.0f + 0.5f;
	projTexCoord.y = LightPos.y / LightPos.w / 2.0f * -1.0f + 0.5f;

	float visibility = 1.0f;

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
		visibility = sum / 16;
	}

	return visibility;
}

float GetAtten(float distance)
{
	return 1.0 / (pow(distance, 2) + distance);
}

#endif