
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float4 main(PS_INTPUT IN) : SV_Target
{
	float4 finalColor = txDiffuse.Sample(samLinear, IN.Tex);

	//finalColor = finalColor / (finalColor + 1.0f);
	finalColor = pow(finalColor, 1.0f / 2.2f);

	return finalColor;
}
