
struct PS_INTPUT
{
	float4 Pos : SV_POSITION;
	float Depth : TEXCOORD1;
};

float4 main(PS_INTPUT IN) : SV_Target
{
	return float4(IN.Pos.z, 0.0f, 0.0f, 1.0f);
}
