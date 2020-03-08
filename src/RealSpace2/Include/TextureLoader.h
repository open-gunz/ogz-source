#pragma once

#include "MUtil.h"

namespace RealSpace2 {

struct TextureInfo
{
	u32 Width;
	u32 Height;
	u32 MipLevels;
	int Format;
};

D3DPtr<struct IDirect3DTexture9> LoadTexture(const void* data, size_t size,
	float sample_ratio,
	TextureInfo& info, const char* ext = nullptr);

}