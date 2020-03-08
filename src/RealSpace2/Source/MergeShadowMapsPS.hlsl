#include "Globals.hlslh"

int NumLights : register(c24);

#define MAKE_SAMPLER(register_num) \
texture txShadowCube##register_num; \
samplerCUBE samShadowCube##register_num : register(s##register_num) = \
sampler_state \
{ \
	Texture = <txShadowCube##register_num>; \
	MinFilter = Point; \
	MagFilter = Point; \
	MipFilter = Point; \
	AddressU = Clamp; \
	AddressV = Clamp; \
};

MAKE_SAMPLER(0);
MAKE_SAMPLER(1);
MAKE_SAMPLER(2);
MAKE_SAMPLER(3);
MAKE_SAMPLER(4);
MAKE_SAMPLER(5);
MAKE_SAMPLER(6);
MAKE_SAMPLER(7);

float4 GetLightDiffuse(samplerCUBE Sampler, int Index, float3 Pos, float3 Normal)
{
	float3 Diff = (float3)Pos - LightPos[Index];
	float DistanceSq = LengthSq(Diff);

	float3 DirToLight = rsqrt(DistanceSq) * Diff;

	float LightAmount = 0;
	LightAmount += (bool)(sqrt(texCUBE(Sampler, Diff)) + 3 < sqrt(DistanceSq)) ? 0 : (1.0);

	if (DistanceSq > AttenuationValues.x)
	{
		if (DistanceSq > AttenuationValues.y)
			LightAmount = 0;
		else
			LightAmount *= (AttenuationValues.z - (DistanceSq - AttenuationValues.x)) / AttenuationValues.z;
	}

	return saturate(dot(-DirToLight, normalize(Normal))) * LightAmount * (1 - LightAmbient);
}

float4 main(
	float3 Pos : TEXCOORD0,
	float3 Normal : NORMAL) : COLOR
{
	float4 Diffuse1 = GetLightDiffuse(samShadowCube2, 0, Pos, Normal);

	return Diffuse1 + LightAmbient;
}