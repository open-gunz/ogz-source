#pragma once

namespace ShaderConstant
{
enum
{
	WorldViewProjection = 0,
	WorldView = 4,
	Projection = 8,
	LightViewProjection = 12,
	Material = 16,
	LightDiffuse,
	LightAmbient,
	CosTheta,
	ShadowMapSize,
	AttenuationValues,
	World,
	LightPos0 = 40,
	LightDir0 = 48,
};
}

namespace ShaderSampler
{
enum
{
	Scene,
	Lightmap,
	Diffuse = 3,
	Shadow0 = 0,
};
}

namespace DeferredShaderConstant
{
enum
{
	WorldViewProjection = 0,
	WorldView = 4,
	World = 8,
	Diffuse = 12,
	Ambient,
	Spec,
	SpecLevel,
	Glossiness,
	Opacity,
	Near,
	Far,
};
}

namespace LightingShaderConstant
{
enum
{
	Proj = 0,
	InvRes = 4,
	Near,
	Far,
	Z,
	Light,
	LightColor,
	InvFocalLen,
};
}

namespace DepthCopyShaderConstant
{
enum
{
	Near,
	Far
};
}

inline void SetVSVector4(UINT Register, const v4& Vector) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(Vector), 1); }
inline void SetVSVector3(UINT Register, const v3& Vector) {
	SetVSVector4(Register, v4{ EXPAND_VECTOR(Vector), 1 }); }
inline void SetVSVector2(UINT Register, const v2& Vector) {
	SetVSVector4(Register, v4{ Vector.x, Vector.y, 1, 1 }); }
inline void SetVSMatrix(UINT Register, const rmatrix& Matrix) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(Matrix), 4); }
inline void SetVSFloat(UINT Register, float Value) {
	RGetDevice()->SetVertexShaderConstantF(Register, static_cast<const float*>(v4{ Value, 0, 0, 0 }), 1); }

inline void SetPSVector4(UINT Register, const v4& Vector) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(Vector), 1); }
inline void SetPSVector3(UINT Register, const v3& Vector) {
	SetPSVector4(Register, v4{ EXPAND_VECTOR(Vector), 1 }); }
inline void SetPSVector2(UINT Register, const v2& Vector) {
	SetPSVector4(Register, v4{ Vector.x, Vector.y, 1, 1 }); }
inline void SetPSMatrix(UINT Register, const rmatrix& Matrix) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(Matrix), 4); }
inline void SetPSFloat(UINT Register, float Value) {
	RGetDevice()->SetPixelShaderConstantF(Register, static_cast<const float*>(v4{ Value, 0, 0, 0 }), 1); }

inline void SetShaderVector4(UINT Register, const v4& Vector) {
	SetVSVector4(Register, Vector);
	SetPSVector4(Register, Vector);
}
inline void SetShaderVector3(UINT Register, const v3& Vector) {
	SetVSVector3(Register, Vector);
	SetPSVector3(Register, Vector);
}
inline void SetShaderVector2(UINT Register, const v2& Vector) {
	SetVSVector2(Register, Vector);
	SetPSVector2(Register, Vector);
}
inline void SetShaderMatrix(UINT Register, const rmatrix& Matrix) {
	SetVSMatrix(Register, Matrix);
	SetPSMatrix(Register, Matrix);
}
inline void SetShaderFloat(UINT Register, float Value) {
	SetVSFloat(Register, Value);
	SetPSFloat(Register, Value);
}