sampler2D depthTexture : register(s0);

float4 main(in float2 TexCoord : TEXCOORD0) : COLOR0
{
	float4 color = tex2D(depthTexture, TexCoord) * 10;
	return float4(color.xxx, 1);
}