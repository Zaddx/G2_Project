// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos  : SV_POSITION;
	float2 uv 	: UV;
	float3 norm : NORM;
};

texture2D textureFile[2] : register(t0);

SamplerState envFilter : register(s0);

// Simple pixel shader, inputs an interpolated vertex color and outputs it to the screen
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color1, color2, colorOut;
	color1 = textureFile[0].Sample(envFilter, input.uv);
	color2 = textureFile[1].Sample(envFilter, input.uv);

	colorOut = lerp(color1, color2, color2.w);
	colorOut = saturate(colorOut);
	
	return colorOut;
}