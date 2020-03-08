#include "Globals.hlslh"

void main(
	float4 inPos               : POSITION,
	float3 inNormal            : NORMAL,
	float2 inTex               : TEXCOORD0,
	float2 inLightmapTex       : TEXCOORD1,
	out float4 outPos          : POSITION,  // Transformed position
	out float2 outTex          : TEXCOORD0, // Texture coord
	out float2 outLightmapTex  : TEXCOORD1, // Lightmap texture coord
	out float3 outWorldPos     : TEXCOORD2, // World space position
	out float3 outNormal       : TEXCOORD3, // World space normal
	out float4 outProjPos      : TEXCOORD4) // Projection space position
{
	outPos = mul(inPos, WorldViewProjection);
	outTex = inTex;
	outLightmapTex = inLightmapTex;
	outWorldPos = (float3)mul(inPos, World);
	outNormal = mul(inNormal, (float3x3)World);
	outProjPos = mul(inPos, WorldViewProjection);
}