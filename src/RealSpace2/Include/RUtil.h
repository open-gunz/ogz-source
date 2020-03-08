#pragma once

#include "RealSpace2.h"
#include "MUtil.h"
#include <type_traits>

using ShaderPair = std::pair<D3DPtr<IDirect3DVertexShader9>, D3DPtr<IDirect3DPixelShader9>>;
using ShaderRefPair = std::pair<const D3DPtr<IDirect3DVertexShader9>&, const D3DPtr<IDirect3DPixelShader9>&>;

inline void DisableShaders()
{
	RGetDevice()->SetVertexShader(nullptr);
	RGetDevice()->SetPixelShader(nullptr);
}

inline ShaderPair SaveAndDisableShaders()
{
	D3DPtr<IDirect3DVertexShader9> PrevVS;
	RGetDevice()->GetVertexShader(MakeWriteProxy(PrevVS));
	D3DPtr<IDirect3DPixelShader9> PrevPS;
	RGetDevice()->GetPixelShader(MakeWriteProxy(PrevPS));

	DisableShaders();

	return{ std::move(PrevVS), std::move(PrevPS) };
}

inline void SetShaders(const ShaderPair& Shaders)
{
	RGetDevice()->SetVertexShader(Shaders.first.get());
	RGetDevice()->SetPixelShader(Shaders.second.get());
}

inline void SetShaders(const ShaderRefPair& Shaders)
{
	RGetDevice()->SetVertexShader(Shaders.first.get());
	RGetDevice()->SetPixelShader(Shaders.second.get());
}

template <typename T> constexpr D3DFORMAT GetD3DFormat();
template <>	constexpr D3DFORMAT GetD3DFormat<u16>() { return D3DFMT_INDEX16; }
template <> constexpr D3DFORMAT GetD3DFormat<u32>() { return D3DFMT_INDEX32; }

template <size_t size>
auto CreateVertexDeclaration(const D3DVERTEXELEMENT9(&Decl)[size])
{
	D3DPtr<IDirect3DVertexDeclaration9> ptr;
	RGetDevice()->CreateVertexDeclaration(Decl, MakeWriteProxy(ptr));
	return ptr;
}