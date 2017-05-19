// Per-pixel color data passed through the pixel shader. 
struct DS_OUTPUT
{
    float4 PositionL : SV_POSITION;
    float2 uv : UV;
    float3 norm : NORMAL;
};

Texture2D textureFile[3] : register(t0);
SamplerState envFilter : register(s0);
 
// Simple pixel shader, inputs an interpolated vertex color and outputs it to the screen 
float4 main(DS_OUTPUT input) : SV_TARGET
{
    // [0] Heightmap
    // [1] Grass
    // [2] Snow
    float4 finalColor;

    float4 heightmap_Color = textureFile[0].SampleLevel(envFilter, input.uv, 0);
    float ratio = heightmap_Color.r;

    float4 grass_Color = textureFile[1].SampleLevel(envFilter, input.uv, 0);
    float4 snow_Color = textureFile[2].SampleLevel(envFilter, input.uv, 0);

    finalColor = lerp(grass_Color, snow_Color, ratio);

    // Read the color from the two images, and the color from the Heightmap
    return finalColor;

}