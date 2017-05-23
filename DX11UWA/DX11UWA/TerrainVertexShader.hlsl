#pragma pack_matrix(row_major) 
 
// A constant buffer that stores the three basic column-major matrices for composing geometry. 
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
}; 
 
// Per-vertex data used as input to the vertex shader. 
struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 uv : UV;
    float3 norm : NORM;
};
 
struct HS_CONTROL_POINT_INPUT
{
    // send local space to Domain
    float4 PositionL : SV_POSITION;
    float2 uv : UV;
    float3 norm : NORMAL;
};

// Simple shader to do vertex processing on the GPU. 
HS_CONTROL_POINT_INPUT main(VertexShaderInput input)
{
    HS_CONTROL_POINT_INPUT output;

  // Transform the vertex position into projected space. 
    output.PositionL = float4(input.pos, 1.0f);
 
  // Pass the color through without modification. 
    output.uv = input.uv.xy;

  // Pass the normals through without modification. 
    output.norm = input.norm;
 
    return output;
}