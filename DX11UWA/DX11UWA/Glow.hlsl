SamplerState pointSampler : register(s0);

struct QuadVertexShaderOutput
{
    float4 pos : SV_Position;
    float2 uv : UV;
    float3 norm : NORM;
};

Texture2D s0 : register(t0);

cbuffer cb0
{
    float2 sampleOffsets[15];
    float4 sampleWeigths[15];
};

float4 Glow(QuadVertexShaderOutput input) : SV_Target
{
    float4 glow = 0.0f;
    float4 color = 0.0f;
    float2 samplePosition;

    for (int sampleIndex = 0; sampleIndex < 15; sampleIndex++)
    {
        // Sample from adjacent points
        samplePosition = input.uv + sampleOffsets[sampleIndex];
        color = s0.Sample(pointSampler, samplePosition);

        glow += sampleWeigths[sampleIndex] * color;
    }

    return glow;
}