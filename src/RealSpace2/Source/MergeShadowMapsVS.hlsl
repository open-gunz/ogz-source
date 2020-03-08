#include "Globals.hlslh"

void main(
	float4 Pos : POSITION,
	float3 Normal : NORMAL,
	out float4 outPos : POSITION,
	out float3 outNormal : NORMAL,
	out float3 outWorldPos : TEXCOORD0)
{
	outPos = mul(Pos, WorldViewProjection);
	outNormal = Normal;
	outWorldPos = (float3)mul(Pos, World);
}