#include "Globals.hlslh"

float4 GetDiffuse(float4 PosLight, float3 PixelDirToLight, float3 Normal, float3 Diff, float DistanceSq)
{
	// Transform from RT space to texture space.
	float2 ShadowTexC = 0.5 * PosLight.xy / PosLight.w + float2(0.5, 0.5);
	ShadowTexC.y = 1.0f - ShadowTexC.y;

	// Transform to texel space
	float2 texelpos = ShadowMapSize * ShadowTexC;

	// Determine the lerp amounts           
	float2 lerps = frac(texelpos);

	// Read in bilerp stamp, doing the shadow checks
	float sourcevals[4];
	sourcevals[0] = (bool)(tex2D(samShadow, ShadowTexC) +
		SHADOW_EPSILON < PosLight.z / PosLight.w) ? 0.0f : 1.0f;
	sourcevals[1] = (bool)(tex2D(samShadow, ShadowTexC + float2(1.0 / ShadowMapSize, 0)) +
		SHADOW_EPSILON < PosLight.z / PosLight.w) ? 0.0f : 1.0f;
	sourcevals[2] = (bool)(tex2D(samShadow, ShadowTexC + float2(0, 1.0 / ShadowMapSize)) +
		SHADOW_EPSILON < PosLight.z / PosLight.w) ? 0.0f : 1.0f;
	sourcevals[3] = (bool)(tex2D(samShadow, ShadowTexC + float2(1.0 / ShadowMapSize, 1.0 / ShadowMapSize)) +
		SHADOW_EPSILON < PosLight.z / PosLight.w) ? 0.0f : 1.0f;

	// Lerp between the shadow values to calculate our light amount
	float LightAmount = lerp(lerp(sourcevals[0], sourcevals[1], lerps.x),
		lerp(sourcevals[2], sourcevals[3], lerps.x),
		lerps.y);

	if (DistanceSq > AttenuationValues.x)
	{
		if (DistanceSq > AttenuationValues.y)
			LightAmount = 0;
		else
			LightAmount *= (AttenuationValues.z - (DistanceSq - AttenuationValues.x)) / AttenuationValues.z;
	}

	// Calculate final diffuse
	return saturate(dot(-PixelDirToLight, normalize(Normal))) * LightAmount * (1 - LightAmbient) + LightAmbient;
}

float4 main(
	float2 Tex         : TEXCOORD0,
	float2 LightmapTex : TEXCOORD1,
	float4 Pos         : TEXCOORD2,
	float3 Normal      : TEXCOORD3,
	float4 PosLight    : TEXCOORD4) : COLOR
{
	float4 Diffuse;

	float3 Diff = (float3)Pos - LightPos[0];
	float DistanceSq = dot(Diff, Diff);

	// Unit vector from the light to this pixel
	float3 Light = rsqrt(DistanceSq) * Diff;

	// Compute diffuse from the light
	if (dot(Light, LightDir[0]) > CosTheta) // Light must face the pixel (within theta)
	{
		// Pixel is in lit area. Find out if it's
		// in shadow using 2x2 percentage closest filtering
		Diffuse = GetDiffuse(PosLight, Light, Normal, Diff, DistanceSq);
	}
	else
	{
		// Pixel is only lit by the ambient lighting
		Diffuse = LightAmbient;
	}

	float4 SceneColor = tex2D(samScene, Tex);
	//float4 LightmapColor = tex2D(samLightmap, LightmapTex);
	//return float4((saturate(SceneColor.rgb * LightmapColor.rgb * 4)), LightmapColor.a);
	return SceneColor * Diffuse;
}