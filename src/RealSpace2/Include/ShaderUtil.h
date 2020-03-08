#pragma once

#include "RNameSpace.h"
#include "MUtil.h"

_NAMESPACE_REALSPACE2_BEGIN

D3DPtr<IDirect3DVertexShader9> CreateVertexShader(const BYTE* Function);
D3DPtr<IDirect3DPixelShader9> CreatePixelShader(const BYTE* Function);

D3DPtr<IDirect3DVertexShader9> CreateVertexShaderFromFile(const char* Filename);
D3DPtr<IDirect3DPixelShader9> CreatePixelShaderFromFile(const char* Filename);

_NAMESPACE_REALSPACE2_END
