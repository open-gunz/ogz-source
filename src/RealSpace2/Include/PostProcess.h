#pragma once

#include "RUtil.h"
#include <vector>

_NAMESPACE_REALSPACE2_BEGIN

struct PostProcessEffect
{
	bool Active{};
	const char* Name{};
	std::vector<ShaderPair> Shaders;
};

class PostProcessManager
{
public:
	void Begin();
	void End();

	void OnInvalidate();
	void OnRestore();

	void AddEffect(PostProcessEffect&& Effect) { Effects.emplace_back(std::move(Effect)); }
	bool EnableEffect(const char* Name, bool Enable);

private:
	void Init();
	void CreateFrameBuffers();

	bool Initialized{};

	std::vector<PostProcessEffect> Effects;

	std::array<D3DPtr<IDirect3DTexture9>, 2> RTs;
	std::array<D3DPtr<IDirect3DSurface9>, 2> RTSurfaces;
	D3DPtr<IDirect3DSurface9> OrigRenderTarget;
};

_NAMESPACE_REALSPACE2_END
