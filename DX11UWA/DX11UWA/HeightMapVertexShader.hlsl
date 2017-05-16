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
    float2 uv : UV;
    float3 norm : NORM;
};
 
// Per-pixel color data passed through the pixel shader. 
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    float3 norm : NORM;
};
 
Texture2D textureFile : register(t0);
SamplerState envFilter : register(s0);

// Simple shader to do vertex processing on the GPU. 
PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);
 
  // Get the color of the texture
    float4 textureColor = textureFile.SampleLevel(envFilter, input.uv, 0);

  // Transform the vertex position into projected space. 
    pos = mul(pos, model);

    // Adjust the y based on the Red value
    pos.y += textureColor.r * 10.0f;

    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.pos = pos;
 
  // Pass the color through without modification. 
    output.uv = input.uv;
    


  // Pass the normals through without modification. 
    output.norm = input.norm;
 
    return output;
}