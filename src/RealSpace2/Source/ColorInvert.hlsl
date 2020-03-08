#include "Globals.hlslh"

float4 main(float2 Tex : TEXCOORD0) : COLOR0
{
	float3 color = 1 - (float3)tex2D(samScene, Tex);
	return float4(color, 1);
}