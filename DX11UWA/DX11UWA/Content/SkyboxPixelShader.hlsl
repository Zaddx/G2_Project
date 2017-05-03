// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos  : SV_POSITION;
	float3 uv 	: UV;
	float3 posL : PL;
};

textureCUBE textureFile : register(t0);

SamplerState envFilter : register(s0);

// Simple pixel shader, inputs an interpolated vertex color and outputs it to the screen
float4 main(PixelShaderInput input) : SV_TARGET
{
	return textureFile.Sample(envFilter, input.posL);
}