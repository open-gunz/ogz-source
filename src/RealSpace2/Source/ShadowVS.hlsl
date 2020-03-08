#include "Globals.hlslh"

void main(
	float4 Pos             : POSITION,
	out float4 outPos      : POSITION,
	out float3 outWorldPos : TEXCOORD0)
{
	outPos = mul(Pos, WorldViewProjection);
	outWorldPos = (float3)mul(Pos, World);
}