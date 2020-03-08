#include "stdafx.h"
#include "Renderer.h"
#include "ShaderGlobals.h"
#include <fstream>
#include "ShaderUtil.h"
#include "VertexTypes.h"
#include "Sphere.h"
#include "RLightList.h"
#include "RS2.h"
// TODO: Remove
#include "../CSCommon/Include/Config.h"

// Shader objects
#include "DeferredVS.h"
#include "DeferredPS.h"
#include "PointLightVS.h"
#include "PointLightPS.h"
#include "DepthCopyVS.h"
#include "DepthCopyPS.h"
#include "VisualizeLinearDepth.h"
#include "Monochrome.h"
#include "ColorInvert.h"

_NAMESPACE_REALSPACE2_BEGIN

static auto CreateScreenSpaceVertexDeclaration()
{
	D3DPtr<IDirect3DVertexDeclaration9> Decl;

	D3DVERTEXELEMENT9 Elements[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, sizeof(float) * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	RGetDevice()->CreateVertexDeclaration(Elements, MakeWriteProxy(Decl));

	return Decl;
}

Renderer::Renderer()
	: LightPos{ 0, 0, 100 }, NearZ{ DEFAULT_NEAR_Z }, FarZ{ DEFAULT_FAR_Z }
{
	if (!GetRS2().UsingD3D9())
		return;

	SetTransform(TransformType::World, GetIdentityMatrix());
	ScreenSpaceVertexDeclaration = CreateScreenSpaceVertexDeclaration();
	CreateShaders();
}

void Renderer::SetScreenSpaceVertexDeclaration()
{
	RGetDevice()->SetVertexDeclaration(ScreenSpaceVertexDeclaration.get());
}

void Renderer::CreateTextures()
{
	if (!DynamicLights)
		return;

	for (size_t i{}; i < std::size(RTs); ++i)
	{
		auto& RT = RTs[i];

		D3DFORMAT Format = i == static_cast<size_t>(RTType::LinearDepth) ? D3DFMT_R32F : D3DFMT_A8R8G8B8;

		auto hr = RGetDevice()->CreateTexture(
			RGetScreenWidth(), RGetScreenHeight(),
			1, D3DUSAGE_RENDERTARGET,
			Format,
			D3DPOOL_DEFAULT,
			MakeWriteProxy(RT),
			nullptr);
		if (FAILED(hr))
		{
			MLog("Renderer::CreateTextures -- Failed to create render target %d, hr = %08X\n", i, hr);
		}
	}
}

void Renderer::CreateShaders()
{
	if (SupportsPixelShaderVersion(3, 0))
	{
		auto AddEffect = [&](const char* Name, auto&& Data) {
			PostProcessEffect Effect;
			Effect.Name = Name;
			Effect.Shaders.emplace_back(ShaderPair{ nullptr, CreatePixelShader(Data) });
			GetRenderer().PostProcess.AddEffect(std::move(Effect));
		};

		AddEffect("Monochrome", MonochromeData);
		AddEffect("ColorInvert", ColorInvertData);
	}

	SupportsDynamicLighting_ = SupportsVertexShaderVersion(3, 0) &&
		SupportsPixelShaderVersion(3, 0) &&
		GetDeviceCaps().NumSimultaneousRTs >= 4 &&
		GetDeviceCaps().MaxStreams >= 4;

	if (!SupportsDynamicLighting())
		return;

	DeferredVS = CreateVertexShader(DeferredVSData);
	DeferredPS = CreatePixelShader(DeferredPSData);
	PointLightVS = CreateVertexShader(PointLightVSData);
	PointLightPS = CreatePixelShader(PointLightPSData);
	DepthCopyVS = CreateVertexShader(DepthCopyVSData);
	DepthCopyPS = CreatePixelShader(DepthCopyPSData);
	VisualizeLinearDepthPS = CreatePixelShader(VisualizeLinearDepthData);
}

void Renderer::OnInvalidate()
{
	SAFE_RELEASE(DeferredVS);
	SAFE_RELEASE(DeferredPS);
	SAFE_RELEASE(PointLightVS);
	SAFE_RELEASE(PointLightPS);
	SAFE_RELEASE(DepthCopyVS);
	SAFE_RELEASE(DepthCopyPS);
	SAFE_RELEASE(VisualizeLinearDepthPS);
	SAFE_RELEASE(OrigRT);
	SAFE_RELEASE(OrigDepthSurface);

	for (auto& RT : RTs)
		SAFE_RELEASE(RT);

	PostProcess.OnInvalidate();
}

void Renderer::OnRestore()
{
	CreateTextures();
	CreateShaders();

	PostProcess.OnRestore();
}

void Renderer::BeginLighting()
{
	if (!DynamicLights)
		return;

	RGetDevice()->SetVertexShader(DeferredVS.get());
	RGetDevice()->SetPixelShader(DeferredPS.get());

	const auto& WorldView = RView;
	SetShaderMatrix(DeferredShaderConstant::WorldView, Transpose(WorldView));
	const auto& Projection = RProjection;
	auto WorldViewProjection = WorldView * RProjection;
	SetShaderMatrix(DeferredShaderConstant::WorldViewProjection, Transpose(WorldViewProjection));

	SetShaderFloat(DeferredShaderConstant::Near, GetNearZ());
	SetShaderFloat(DeferredShaderConstant::Far, GetFarZ());

	RGetDevice()->GetRenderTarget(0, MakeWriteProxy(OrigRT));
	for (size_t i{}; i < std::size(RTs); ++i)
	{
		D3DPtr<IDirect3DSurface9> Surface;
		RTs[i]->GetSurfaceLevel(0, MakeWriteProxy(Surface));
		RGetDevice()->SetRenderTarget(i, Surface.get());
	}
}

void Renderer::EndLighting(const std::vector<struct RLIGHT>& Lights)
{
	if (!DynamicLights)
		return;

	DrawLighting(Lights);

	if (ShowRTsEnabled)
		ShowRTs();
}

void Renderer::SetAlphaBlending(bool b)
{
	if (AlphaBlending == b)
		return;

	AlphaBlending = b;
}

bool Renderer::SetDrawShadows(bool b)
{
	if (DrawingShadows == b)
		return true;

	DrawingShadows = b;

	return true;
}

void Renderer::SetDynamicLights(bool b)
{
	DynamicLights = b;
}

void Renderer::UpdateRTs()
{
	if (!RTs[0])
		CreateTextures();
}

void DrawQuad(const v2& p1, const v2& p2)
{
	struct VertexType
	{
		float x, y, z, rhw;
		float u, v;
	};

	ScreenSpaceTexVertex Vertices[] = {
		{ p1.x - 0.5f, p1.y - 0.5f, 0, 1, 0, 0 },
		{ p1.x - 0.5f, p2.y - 0.5f, 0, 1, 0, 1 },
		{ p2.x - 0.5f, p2.y - 0.5f, 0, 1, 1, 1 },
		{ p2.x - 0.5f, p1.y - 0.5f, 0, 1, 1, 0 },
	};

	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &Vertices, sizeof(VertexType));
}

void DrawFullscreenQuad()
{
	auto Width = (float)RGetScreenWidth();
	auto Height = (float)RGetScreenHeight();
	DrawQuad({ 0, 0 }, { Width, Height });
}

static void DrawAmbient(LPDIRECT3DTEXTURE9 Ambient)
{
	RGetDevice()->SetTexture(0, Ambient);
	DrawFullscreenQuad();
}

static bool IsSphereInsideFrustum(const v3& Center, float Radius, const rplane(&Frustum)[6])
{
	for (auto& Plane : Frustum)
		if (DotProduct(Center, v3{ EXPAND_VECTOR(Plane) }) + Plane.d < -Radius)
			return false;
	return true;
}

void Renderer::DrawLighting(const std::vector<RLIGHT>& Lights)
{
	RGetDevice()->SetRenderTarget(0, OrigRT.get());
	OrigRT.reset();
	for (size_t i = 1; i < std::size(RTs); ++i)
		RGetDevice()->SetRenderTarget(i, nullptr);

	RGetDevice()->SetVertexShader(nullptr);
	RGetDevice()->SetPixelShader(nullptr);

	// Enable additive blending
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	RGetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	// Disable depth writing and testing
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	DrawAmbient(GetRT(RTType::Ambient).get());

	RGetDevice()->SetVertexShader(PointLightVS.get());
	RGetDevice()->SetPixelShader(PointLightPS.get());

	for (size_t i{}; i < std::size(RTs); ++i)
		RGetDevice()->SetTexture(i, RTs[i].get());

	SetShaderMatrix(LightingShaderConstant::Proj, Transpose(RProjection));
	v2 InvRes{ 1.0f / RGetScreenWidth(), 1.0f / RGetScreenHeight() };
	SetShaderVector2(LightingShaderConstant::InvRes, InvRes);
	v2 InvFocalLen{ 1.0f / RProjection._11, 1.0f / RProjection._22 };
	SetShaderVector2(LightingShaderConstant::InvFocalLen, InvFocalLen);
	SetShaderFloat(LightingShaderConstant::Near, GetNearZ());
	SetShaderFloat(LightingShaderConstant::Far, GetFarZ());

	RGetDevice()->SetFVF(D3DFVF_XYZ);

	for (auto& Light : Lights)
	{
		if (!isInViewFrustum(Light.Position, Light.fAttnEnd, RViewFrustum))
			continue;

		auto& pos = Light.Position * RView;
		SetShaderVector4(LightingShaderConstant::Light, { EXPAND_VECTOR(pos), Light.fAttnEnd });
		SetShaderVector3(LightingShaderConstant::LightColor, Light.Color);

		if (Magnitude(pos) < Light.fAttnEnd * 1.3 + GetNearZ())
		{
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}
		else
		{
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}

		RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST,
			rsx::data::nvert / 3, rsx::data::sphere, 3 * sizeof(float));
	}

	// Reset states
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, true);

	RGetDevice()->SetVertexShader(nullptr);
	RGetDevice()->SetPixelShader(nullptr);
}

void Renderer::ShowRTs()
{
	auto ShowRT = [&](auto& RT, auto&& p1, auto&& p2) {
		RGetDevice()->SetTexture(0, RT.get());
		DrawQuad(p1, p2);
	};

	auto ShowDepth = [&](auto& RT, auto&& p1, auto&& p2) {
		RGetDevice()->SetPixelShader(VisualizeLinearDepthPS.get());
		SetScreenSpaceVertexDeclaration();
		ShowRT(RT, p1, p2);
		RGetDevice()->SetPixelShader(nullptr);
	};

	for (size_t i{}; i < std::size(RTs); ++i)
	{
		auto Width = float(RGetScreenWidth());
		auto Height = float(RGetScreenHeight());
		v2 p1{ Width / 2 + (i % 2) * Width / 4, Height / 2 + (i >= 2) * Height / 4 };
		v2 p2{ p1 + v2{ Width / 4, Height / 4 } };
		if (i == static_cast<size_t>(RTType::LinearDepth))
			ShowDepth(RTs[i], p1, p2);
		else
			ShowRT(RTs[i], p1, p2);
	}
}

_NAMESPACE_REALSPACE2_END