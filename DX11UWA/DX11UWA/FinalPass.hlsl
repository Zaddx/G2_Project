static const float glowScale = 4.0f;    // Controls glow intensity

SamplerState pointSampler : register(s0);
SamplerState linearSampler : register(s1);

struct QuadVertexShaderOutput
{
    float4 pos  : SV_Position;
    float2 uv   : UV;
    float3 norm : NORM;
};

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);

float4 FinalPass(QuadVertexShaderOutput input) : SV_TARGET
{
    float4 sceneColor = s0.Sample(pointSampler, input.uv);

    float3 glowColor = s1.Sample(linearSampler, input.uv).rgb;

    sceneColor.rgb += glowScale * glowColor;
    sceneColor.a = 1.0f;

    return sceneColor;
}