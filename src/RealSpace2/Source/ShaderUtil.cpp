#include "stdafx.h"
#include "RealSpace2.h"
#include "MUtil.h"
#include "MFile.h"

using namespace RealSpace2;

_NAMESPACE_REALSPACE2_BEGIN

template <typename T, typename fn_t>
static D3DPtr<T> CreateShader(const DWORD* Function, fn_t CreateShaderFunc)
{
	D3DPtr<T> Shader;
	auto hr = (RGetDevice()->*CreateShaderFunc)(Function, MakeWriteProxy(Shader));
	if (FAILED(hr))
	{
		MLog("Failed to create shader! Error code = %d\n", hr);
		return nullptr;
	}

	return Shader;
}

D3DPtr<IDirect3DVertexShader9> CreateVertexShader(const BYTE* Function) {
	return CreateShader<IDirect3DVertexShader9>(reinterpret_cast<const DWORD*>(Function), &IDirect3DDevice9::CreateVertexShader);
}
D3DPtr<IDirect3DPixelShader9> CreatePixelShader(const BYTE* Function) {
	return CreateShader<IDirect3DPixelShader9>(reinterpret_cast<const DWORD*>(Function), &IDirect3DDevice9::CreatePixelShader);
}
template <typename T, typename fn_t>
static D3DPtr<T> CreateShaderFromFile(const char* Filename, fn_t CreateShaderFunc)
{
	MFile::File File{ Filename };

	const auto Size64 = File.size();
	assert(Size64 <= (std::numeric_limits<size_t>::max)());
	const auto Size = static_cast<size_t>(Size64);

	auto Buffer = std::make_unique<char[]>(Size);
	File.read(Buffer.get(), Size);

	auto Function = reinterpret_cast<const DWORD*>(Buffer.get());

	return CreateShader<T>(Function, CreateShaderFunc);
}

D3DPtr<IDirect3DVertexShader9> CreateVertexShaderFromFile(const char* Filename) {
	return CreateShaderFromFile<IDirect3DVertexShader9>(Filename, &IDirect3DDevice9::CreateVertexShader);
}
D3DPtr<IDirect3DPixelShader9> CreatePixelShaderFromFile(const char* Filename) {
	return CreateShaderFromFile<IDirect3DPixelShader9>(Filename, &IDirect3DDevice9::CreatePixelShader);
}

_NAMESPACE_REALSPACE2_END