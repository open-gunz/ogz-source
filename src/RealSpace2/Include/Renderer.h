#pragma once

#include "RealSpace2.h"
#include "PostProcess.h"

_NAMESPACE_REALSPACE2_BEGIN

enum class TransformType
{
	World,
	View,
	Projection,
	End,
};

class Renderer
{
public:
	Renderer();

	void OnInvalidate();
	void OnRestore();

	void BeginLighting();
	void EndLighting(const std::vector<struct RLIGHT>&);

	void UpdateRTs();

	void SetNearZ(float f) { NearZ = f; }
	void SetFarZ(float f) { FarZ = f; }

	float GetNearZ() const { return NearZ; }
	float GetFarZ() const { return FarZ; }

	void SetAlphaBlending(bool b);
	auto IsAlphaBlendingEnabled() const { return AlphaBlending; }

	bool SetDrawShadows(bool b);
	auto IsDrawingShadows() const { return DrawingShadows; }

	void SetDynamicLights(bool b);
	bool GetDynamicLights() const { return DynamicLights; }
	
	bool SupportsDynamicLighting() const { return SupportsDynamicLighting_; }

	void SetTransform(TransformType t, const rmatrix& mat) { GetTransform(t) = mat; }

	void SetScreenSpaceVertexDeclaration();

	D3DPtr<IDirect3DVertexShader9> DeferredVS;
	D3DPtr<IDirect3DPixelShader9> DeferredPS;
	D3DPtr<IDirect3DVertexShader9> PointLightVS;
	D3DPtr<IDirect3DPixelShader9> PointLightPS;
	D3DPtr<IDirect3DVertexShader9> DepthCopyVS;
	D3DPtr<IDirect3DPixelShader9> DepthCopyPS;
	D3DPtr<IDirect3DPixelShader9> VisualizeLinearDepthPS;

	bool ShowRTsEnabled{};
	PostProcessManager PostProcess;

private:
	void CreateTextures();
	void CreateShaders();

	void ShowRTs();

	void DrawLighting(const std::vector<struct RLIGHT>&);

	rmatrix& GetTransform(TransformType t) { return Transforms[static_cast<size_t>(t)]; }

	bool AlphaBlending{};
	bool DrawingShadows{};
	bool CanRenderWithShader{};
	bool DynamicLights{};
	bool SupportsDynamicLighting_{};

	float NearZ{}, FarZ{};

	rmatrix Transforms[static_cast<size_t>(TransformType::End)];
	v3 LightPos;
	v3 LightDir;

	D3DPtr<IDirect3DVertexDeclaration9> VertexDeclaration;
	D3DPtr<IDirect3DVertexDeclaration9> ScreenSpaceVertexDeclaration;

	enum class RTType
	{
		Color,
		Normal,
		LinearDepth,
		Ambient,
		Max,
	};
	D3DPtr<IDirect3DTexture9> RTs[static_cast<size_t>(RTType::Max)];
	auto& GetRT(RTType Index) const { return RTs[static_cast<size_t>(Index)]; }

	D3DPtr<IDirect3DSurface9> OrigRT;
	D3DPtr<IDirect3DSurface9> OrigDepthSurface;
};

void DrawQuad(const v2& p1, const v2& p2);
void DrawFullscreenQuad();


_NAMESPACE_REALSPACE2_END