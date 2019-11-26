
TextureCube txCube : register(t10);
SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
	float4 WorldPos : TEXCOORD1;
};

// Pixel Shader
float4 main( VS_OUTPUT input ) : SV_Target
{
	float PI = 3.14159265359f;

	float3 N = normalize(input.WorldPos);

	float3 irradiance = 0.0f;

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025f;
	float nrSamples = 0.0f;
	float3 tangentSample = 0.0f;
	float3 sampleVec = 0.0f;

	
	//[fastopt]
	for (float phi = 0.0f; phi < 2.0 * PI; phi += sampleDelta)
	{
		
		//[fastopt]
		for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
		{
			tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += txCube.SampleLevel(samLinear, sampleVec, 0.0f).xyz * cos(theta) * sin(theta);

			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0f / nrSamples);

	float4 finalColor = 0;
	finalColor.xyz = irradiance;
	finalColor.w = 1.0f;
	return finalColor;
}
