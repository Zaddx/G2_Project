// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
    // Local space from Vertex Shader
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

float g_TessellationFactor = 15.0f;

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Assign tessellation factors - in this case use a global
    // tessellation factor for all edges and the inside. These are
    // constant for the wole mesh.
    Output.EdgeTessFactor[0] = g_TessellationFactor;
    Output.EdgeTessFactor[1] = g_TessellationFactor;
    Output.EdgeTessFactor[2] = g_TessellationFactor;
    Output.InsideTessFactor = g_TessellationFactor;

	return Output;
}

[domain("tri")]                     // tessellator builds triangles
[partitioning("fractional_odd")]    // type of division
[outputtopology("triangle_cw")]     // topology of output
[outputcontrolpoints(3)]            // num CP sent to Domain Shader
// points to HLSL function which computes tesselation amount
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.PositionL = ip[i].PositionL;
    Output.uv = ip[i].uv;
    Output.norm = ip[i].norm;

	return Output;
}
