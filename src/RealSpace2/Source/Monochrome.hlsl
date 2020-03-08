#include "Globals.hlslh"

static const float3 LuminanceConv = { 0.2125f, 0.7154f, 0.0721f };

float4 main(float2 Tex : TEXCOORD0) : COLOR0
{
	return dot((float3)tex2D(samScene, Tex), LuminanceConv);
}