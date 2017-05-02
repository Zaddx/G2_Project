// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos  : SV_POSITION;
	float2 uv 	: UV;
	float3 norm : NORM;
};

texture2D textureFile : register(t0);

SamplerState envFilter : register(s0);

// Simple pixel shader, inputs an interpolated vertex color and outputs it to the screen
float4 main(PixelShaderInput input) : SV_TARGET
{
	return textureFile.Sample(envFilter, input.uv);
}