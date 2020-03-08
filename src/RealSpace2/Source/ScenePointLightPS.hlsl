#include "Globals.hlslh"

float4 main(
	float2 Tex         : TEXCOORD0,
	float2 LightmapTex : TEXCOORD1,
	float3 WorldPos    : TEXCOORD2,
	float3 Normal      : TEXCOORD3,
	float4 Pos         : TEXCOORD4) : COLOR
{
	float3 Diff = (float3)WorldPos - LightPos[0];
	float DistanceSq = LengthSq(Diff);

	float3 DirToLight = rsqrt(DistanceSq) * Diff;

	float LightAmount = 0;
	float Offset = 1 / ShadowMapSize;
	for (int x = -1; x <= 1; ++x)
		for (int y = -1; y <= 1; ++y)
			LightAmount += (bool)(texCUBE(samShadowCube, float3(Diff.x + x * Offset, Diff.y + x * Offset, Diff.z)) + 3 < sqrt(DistanceSq)) ? 0 : (1.0f / 6);

	if (DistanceSq > AttenuationValues.x)
	{
		if (DistanceSq > AttenuationValues.y)
			LightAmount = 0;
		else
			LightAmount *= (AttenuationValues.z - (DistanceSq - AttenuationValues.x)) / AttenuationValues.z;
	}

	float4 SceneColor = tex2D(samScene, Tex);

	// Calculate final diffuse
	float4 Diffuse = LightAmount * (1 - LightAmbient) + LightAmbient;

	return SceneColor * Diffuse;
}