#pragma once

#include <vector>
#include <map>
#include "MUtil.h"

#define MAX_LIGHT 3
#define UPDATE_LIGHT0 0x1
#define UPDATE_LIGHT1 0x2
#define UPDATE_MATERIAL 0x4

enum ESHADER
{
	SHADER_SKIN,
	SHADER_SKIN_SPEC,
};

class RShaderMgr
{
public:
	RShaderMgr();
	~RShaderMgr();
	
	void OnInvalidate();
	void OnRestore();

	LPDIRECT3DVERTEXSHADER9 getShader(int i_);
	LPDIRECT3DVERTEXDECLARATION9 getShaderDecl(int i);
	void setMtrl(color_r32& rColor_, float fVisAlpha_);
	void setMtrl(class RMtrl* pMtrl_, float fVisAlpha_);
	void setLight(int iLignt_, D3DLIGHT9* pLight_);
	void setAmbient(u32 value_);
	void LightEnable(int iLignt_, bool bEnable_);

	void Update();

	bool Initialize();

	void SetDisable();
	bool SetEnable();

	static RShaderMgr* getShaderMgr();

	static RShaderMgr msInstance;
	static bool mbUsingShader;
	static RMtrl* mpMtrl;
	static D3DLIGHT9 mLight[MAX_LIGHT];

private:
	void init();

	std::vector<D3DPtr<IDirect3DVertexShader9>> m_ShaderVec;
	std::vector<D3DPtr<IDirect3DVertexDeclaration9>> m_ShaderDeclVec;

	bool	mbLight[2];
};

inline RShaderMgr* RGetShaderMgr() { return RShaderMgr::getShaderMgr(); }
