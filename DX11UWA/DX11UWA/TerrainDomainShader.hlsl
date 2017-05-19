#pragma pack_matrix(row_major) 

cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
}

struct DS_OUTPUT
{
    float4 PositionL : SV_POSITION;
    float2 uv : UV;
    float3 norm : NORMAL;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	// send local space to Domain
    float4 PositionL : SV_POSITION;
    float2 uv : UV;
    float3 norm : NORMAL;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

Texture2D HeightMap : register(t0);
SamplerState SampleLinear : register(s0);

float Scale = 100.0f;
float Bias = 0.0f;

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	//Output.PositionL = float4(
	//	patch[0].PositionL * domain.x + patch[1].PositionL * domain.y + patch[2].PositionL * domain.z, 1);

    float3 worldPos =
        domain.x * patch[0].PositionL +
        domain.y * patch[1].PositionL +
        domain.z * patch[2].PositionL;

    Output.uv =
        domain.x * patch[0].uv +
        domain.y * patch[1].uv;

    float3 normal =
        domain.x * patch[0].norm +
        domain.y * patch[1].norm +
        domain.z * patch[2].norm;  

    float Height = HeightMap.SampleLevel(SampleLinear, Output.uv.xy, 0).r;

    Height *= Scale;
    Height += Bias;
    float3 Direction = -normal;     // direction is opposite normal

    // translate the position
    worldPos += Direction * Height;

    float4 pos = mul(float4(worldPos.xyz, 1.0f), world);
    pos = mul(float4(pos.xyz, 1.0f), view);
    Output.PositionL = mul(pos, proj);

	return Output;
}
