#pragma once

#include "GlobalTypes.h"
#include <d3d9.h>

struct ScreenSpaceColorVertex
{
	float x, y, z, rhw;
	u32 color;
};

constexpr u32 ScreenSpaceColorFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

struct ScreenSpaceTexVertex
{
	float x, y, z, rhw;
	float tu, tv;
};

constexpr u32 ScreenSpaceTexFVF = D3DFVF_XYZRHW | D3DFVF_TEX1;

struct ScreenSpaceColorTexVertex
{
	v4 pos;
	u32 color;
	float tu, tv;
};

constexpr u32 ScreenSpaceColorTexFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

struct WorldSpaceColorVertex
{
	float x, y, z;
	u32 color;
};

constexpr u32 WorldSpaceColorFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

struct WorldSpaceTexVertex
{
	v3 pos;
	float tu, tv;
};

constexpr u32 WorldSpaceTexFVF = D3DFVF_XYZ | D3DFVF_TEX1;