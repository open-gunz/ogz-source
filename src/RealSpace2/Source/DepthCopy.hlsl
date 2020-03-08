float g_Near : register(c0);
float g_Far : register(c1);

sampler2D depthTexture : register(s0);

struct VS_OUTPUT
{
	float4 position   : POSITION;
	float2 texCoord   : TEXCOORD0;
};

VS_OUTPUT vs_main(float4 position : POSITION, float2 texCoord : TEXCOORD0)
{
	VS_OUTPUT Out;
	Out.position = float4(position.xyz, 1.0);
	Out.texCoord = texCoord;
	return Out;
}

struct PS_OUTPUT
{
	float4 color: COLOR;
	float depth : DEPTH0;
};

float depthFromLinear(float lin_depth, float near, float far)
{
	float z = lin_depth *(far - near) + near;
	return lin_depth * far / z;
}

PS_OUTPUT ps_main(in VS_OUTPUT In)
{
	PS_OUTPUT Out;
	float z = tex2D(depthTexture, In.texCoord).x;
	z = depthFromLinear(z, g_Near, g_Far);

	Out.depth = z;
	Out.color = 1;
	return Out;
}