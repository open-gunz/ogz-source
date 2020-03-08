#include "stdafx.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "RS2.h"

namespace RealSpace2 {

void PostProcessManager::Begin()
{
	if (!Initialized) {
		return;
	}

	// Save the original render target (the frame buffer) and replace it with our own,
	// so that we can capture the drawn world as a texture for further manipulation.
	RGetDevice()->GetRenderTarget(0, MakeWriteProxy(OrigRenderTarget));
	RGetDevice()->SetRenderTarget(0, RTSurfaces[0].get());
	RClear();
}

void PostProcessManager::End()
{
	if (!Initialized) {
		return;
	}

	// RT currently being rendered to.
	int CurRTIndex{};
	for (size_t EffectIndex = 0; EffectIndex < std::size(Effects); ++EffectIndex)
	{
		auto&& Effect = Effects[EffectIndex];
		if (!Effect.Active) {
			continue;
		}

		// Render each pass.
		for (size_t ShaderIndex = 0; ShaderIndex < std::size(Effect.Shaders); ++ShaderIndex)
		{
			// Swap RTs.
			// Previous RT will now be used as the texture,
			// and the other RT will be used as the new render target.
			RGetDevice()->SetTexture(0, RTs[CurRTIndex].get());
			CurRTIndex = !CurRTIndex;
			RGetDevice()->SetRenderTarget(0, RTSurfaces[CurRTIndex].get());

			auto&& Shaders = Effect.Shaders[ShaderIndex];

			// Disable Z-testing and writing, since we just
			// want the pixel shader to process every pixel.
			RGetDevice()->SetRenderState(D3DRS_ZENABLE, false);
			RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, false);
			RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

			// Draw a quad over the entire screen.
			GetRenderer().SetScreenSpaceVertexDeclaration();
			SetShaders(Shaders);
			DrawFullscreenQuad();
		}
	}

	DisableShaders();

	RGetDevice()->StretchRect(RTSurfaces[CurRTIndex].get(), nullptr,
		OrigRenderTarget.get(), nullptr, D3DTEXF_NONE);
	// We've applied all the effects now.
	// Final step is resetting the original render target
	// and rendering the final results to it.
	RGetDevice()->SetRenderTarget(0, OrigRenderTarget.get());

	// Release the render target pointer that we acquired in Begin.
	OrigRenderTarget.reset();
}

void PostProcessManager::OnInvalidate()
{
	if (!Initialized) {
		return;
	}

	for (auto&& ptr : RTs) {
		ptr.reset();
	}
	for (auto&& ptr : RTSurfaces) {
		ptr.reset();
	}
}

void PostProcessManager::OnRestore()
{
	if (!Initialized) {
		return;
	}

	CreateFrameBuffers();
}

bool PostProcessManager::EnableEffect(const char * Name, bool Enable)
{
	if (!Name) {
		assert(!"Name must be non-null");
		return false;
	}

	for (auto&& Effect : Effects)
	{
		if (!_stricmp(Name, Effect.Name))
		{
			Effect.Active = Enable;
			if (Enable && !Initialized) {
				Init();
			}
			return true;
		}
	}

	return false;
}

void PostProcessManager::Init()
{
	Initialized = true;
	CreateFrameBuffers();
}

void PostProcessManager::CreateFrameBuffers()
{
	for (auto&& ptr : RTs) {
		if (ptr) {
			continue;
		}

		const auto Width = RGetScreenWidth();
		const auto Height = RGetScreenHeight();
		const auto Levels = 1;
		const auto Usage = D3DUSAGE_RENDERTARGET;
		const auto Format = RGetPixelFormat();
		const auto Pool = D3DPOOL_DEFAULT;
		RGetDevice()->CreateTexture(Width, Height, Levels, Usage, Format, Pool, MakeWriteProxy(ptr), nullptr);
	}

	for (size_t i = 0; i < std::size(RTs); ++i) {
		RTs[i]->GetSurfaceLevel(0, MakeWriteProxy(RTSurfaces[i]));
	}
}

}
