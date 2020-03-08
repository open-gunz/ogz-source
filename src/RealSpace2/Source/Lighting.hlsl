struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 pos      : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 Color : COLOR0;
};

float4x4 g_Proj : register(c0);

float2 g_InvRes : register(c4);
float g_Near    : register(c5);
float g_Far     : register(c6);
float g_Z       : register(c7);

float4 g_Light       : register(c8);
float3 g_LightColor  : register(c9);
float2 g_InvFocalLen : register(c10);

sampler2D diffuseTexture : register(s0);
sampler2D normalTexture  : register(s1);
sampler2D depthTexture   : register(s2);
sampler2D ambientTexture : register(s3);

// Ambient

VS_OUTPUT vs_ambient(float4 Position : POSITION)
{
	VS_OUTPUT Out;
	Out.Position = float4(Position.xyz, 1.0);
	Out.pos = Out.Position;
	return Out;
}

PS_OUTPUT ps_ambient(in VS_OUTPUT In)
{
	PS_OUTPUT Out;
	float2 texCoord = (In.pos.xy / In.pos.w) * 0.5 + 0.5;
	texCoord.y = -texCoord.y;
	texCoord += g_InvRes * 0.5;

	Out.Color = float4(tex2D(ambientTexture, texCoord).xyz, 1.0);
	return Out;
}

// Point light

VS_OUTPUT vs_point_light(float4 Position : POSITION)
{
	VS_OUTPUT Out;

	Position.xyz = Position.xyz*(g_Light.w) + g_Light.xyz;
	Out.Position = mul(Position, g_Proj);
	Out.pos = Out.Position;

	return Out;
}

PS_OUTPUT ps_point_light(in VS_OUTPUT In)
{
	PS_OUTPUT Out;

	float2 h = (In.pos.xy / In.pos.w);
	float2 texCoord = h * 0.5 + 0.5;
	texCoord.y = -texCoord.y;
	texCoord += g_InvRes * 0.5;

	float z = tex2D(depthTexture, texCoord).x;
	float3 pos;

	pos.z = z*(g_Far - g_Near) + g_Near;
	pos.xy = (In.pos.xy / In.pos.w) * g_InvFocalLen * pos.z;


	float3 light_dir = normalize(g_Light.xyz - pos);
	float att = length(g_Light.xyz - pos) / g_Light.w;

	att = 1 - att*att;
	att = saturate(att);


	float4 normal_pixel = tex2D(normalTexture, texCoord);
	float3 normal = normal_pixel.xyz * 2 - 1;
	float NdotL = saturate(dot(normal, light_dir));

	float4 diffuse_pixel = tex2D(diffuseTexture, texCoord);
	float specular_power = normal_pixel.w * 255;
	float specular_mask = diffuse_pixel.w;
	float3 halfway = normalize(normalize(-pos) + light_dir);

	float3 diffuse = diffuse_pixel.xyz*NdotL;
	float3 specular = specular_mask * pow(saturate(dot(halfway, normal)), specular_power);

	Out.Color = float4((diffuse + specular)*g_LightColor*att, 1.0);
	return Out;
}