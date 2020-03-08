float4x4 WorldViewProj : WORLDVIEWPROJECTION : register(c0);
float4x4 WorldView : register(c4);

// Unused stuff
float4x4 World     : register(c8);
float3 Diffuse   : register(c12);
float3 Ambient   : register(c13);
float3 Spec      : register(c14);
float SpecLevel  : register(c15);
float Glossiness : register(c16);

// Used stuff
float Opacity    : register(c17);
float Near       : register(c18);
float Far        : register(c19);

sampler2D diffuseTexture   : register(s0);
sampler2D normalTexture    : register(s1);
sampler2D specularTexture  : register(s2);
sampler2D opacityTexture   : register(s3);
sampler2D selfIllumTexture : register(s4);

//---------------------------------------
//	Vertex
//---------------------------------------

struct VS_OUTPUT
{
	float4 Position   : POSITION;
	float3 Normal	  : TEXCOORD1;
	float2 Texture    : TEXCOORD2;
	float3 Tangent    : TEXCOORD3;
	float3 Binormal   : TEXCOORD4;
	float4 viewpos    : TEXCOORD5;
};

VS_OUTPUT vs_main(
	float4 Position	: POSITION,
	float3 Normal   : NORMAL,
	float2 Texture  : TEXCOORD0,
	float4 Tangent  : TANGENT
	)
{
	VS_OUTPUT Out;
	Out.Position = mul(Position, WorldViewProj);
	Out.viewpos = mul(Position, WorldView);

	Out.Normal = normalize(mul(Normal, (float3x3) WorldView));
	Out.Texture = Texture;

	Out.Tangent = normalize(mul(Tangent.xyz, (float3x3) WorldView));
	Out.Binormal = normalize(cross(Out.Tangent, Out.Normal) * Tangent.w);

	return Out;
}

//---------------------------------------
//	Pixel
//---------------------------------------

struct PS_OUTPUT
{
	float4 Diffuse   : COLOR0;
	float4 Normal    : COLOR1;
	float4 Depth     : COLOR2;
	float4 Ambient   : COLOR3;
};

PS_OUTPUT ps_main(in VS_OUTPUT In)
{
	PS_OUTPUT Out;

	float opacity = tex2D(opacityTexture, In.Texture).w;
	opacity = max(Opacity, opacity);
	clip(opacity - 0.4);

	float3 normal = normalize(2 * tex2D(normalTexture, In.Texture).xyz - 1);

	In.Normal = normal.z * In.Normal + normal.x * In.Tangent + normal.y * In.Binormal;
	normal = normalize(In.Normal);

	float3 color = tex2D(diffuseTexture, In.Texture).xyz;
	float specular = 0;
	// SpecLevel == 0 means no specular texture
	if (SpecLevel <= 0.)
		specular = 0;
	else
		specular = tex2D(specularTexture, In.Texture).w;

	Out.Diffuse.xyz = tex2D(diffuseTexture, In.Texture).xyz;
	Out.Diffuse.w = specular;
	Out.Normal.xyz = normal*0.5 + 0.5;
	Out.Normal.w = SpecLevel;

	float d = (In.viewpos.z - Near) / (Far - Near);
	Out.Depth = float4(d, d, d, 1.0);
	Out.Ambient = float4(tex2D(selfIllumTexture, In.Texture).xyz + Out.Diffuse.xyz*0.3, 1.0);
	return Out;
}