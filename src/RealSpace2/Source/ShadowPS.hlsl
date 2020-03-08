#include "Globals.hlslh"

float4 main(float3 Pos : TEXCOORD0) : COLOR
{
	return LengthSq(Pos - LightPos[0]);
}