// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 wpos : WORLD_POS;
	float3 uv : UV;
	float3 norm : NORM;
};

cbuffer Directional_Light : register(b0)
{
	float4 direction_directional;
	float4 color_directional;
}

cbuffer Point_Light : register(b1)
{
	float4 position_point;
	float4 color_point;
	float4 radius_point;
} 

cbuffer Spot_Light : register(b2)
{
	float4 position_spot;
	float4 color_spot;
	float4 cone_direction;
	float4 cone_ratio;
	float4 inner_cone_ratio;
	float4 outer_cone_ratio;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	// Overall result for the new color
	float3 overall_result = { 0.0f, 0.0f, 0.0f };

	float3 surfacePosition = input.wpos;
	float3 surfaceNormal = input.norm;
	float3 surfaceColor = input.uv;

	// Directional Light
	{
		float3 lightDirection = -direction_directional.xyz;
		float3 lightColor = color_directional.xyz;

		float dot_result;
		float3 result;

		dot_result = saturate(dot(lightDirection, surfaceNormal));
		result = dot_result * lightColor * surfaceColor;

		overall_result = result;
	}

	// Point Light
	{
		float attenuation;
		float3 lightDirection;
		float3 result;

		float3 lightPosition = position_point.xyz;
		float3 lightColor = color_point.xyz;

		float3 light_minus_surface = lightPosition - surfacePosition;
		float light_minus_surface_length = length(light_minus_surface);

		lightDirection = normalize(light_minus_surface);

		float dot_result = saturate(dot(lightDirection, surfaceNormal));

		attenuation = 1.0f - saturate(light_minus_surface_length / radius_point.x);
		result = attenuation * lightColor * dot_result;

		overall_result += result;
	}
	 
	// Spot Light
	{
		float3 lightDir;
		float3 result;

		float3 lightPos = position_spot.xyz;
		float3 lightColor = color_spot.xyz;
		float3 coneDir = cone_direction.xyz;

		float surfaceRatio;
		float spotFactor;
		float lightRatio;
		float attenuation;

		float coneRatio = cone_ratio.x;
		float innerConeRatio = inner_cone_ratio.x;
		float outerConeRatio = outer_cone_ratio.x;

		lightDir = normalize((lightPos - surfacePosition));
		surfaceRatio = saturate(dot(-lightDir, coneDir));
		spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
		lightRatio = saturate(dot(lightDir, surfaceNormal));
		attenuation = 1.0f - saturate((innerConeRatio - surfaceRatio) / (innerConeRatio - outerConeRatio));
		result = spotFactor * attenuation * lightColor * surfaceColor;

		overall_result += result;
	}

	return float4(overall_result, 1.0f);
}