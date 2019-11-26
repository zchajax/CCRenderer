
TextureCube txCube : register(t10);
SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
};

float4 main( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = 0;
	finalColor += txCube.Sample(samLinear, input.Tex);
	return finalColor;
}
