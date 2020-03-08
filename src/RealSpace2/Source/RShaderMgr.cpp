#include "stdafx.h"
#include <d3d9.h>
#include "RShaderMgr.h"
#include "RealSpace2.h"
#include "rmtrl.h"
#include "rmeshutil.h"
#include "MDebug.h"
#include "ShaderUtil.h"

// skin shader object
#include "skin.h"

RShaderMgr RShaderMgr::msInstance;
bool RShaderMgr::mbUsingShader = true;
RMtrl* RShaderMgr::mpMtrl = 0;
D3DLIGHT9 RShaderMgr::mLight[MAX_LIGHT];

RShaderMgr::RShaderMgr()
{
	if (!mpMtrl)
		mpMtrl = new RMtrl;

	mbLight[0] = false;
	mbLight[1] = false;
	mbUsingShader = false;
}

RShaderMgr::~RShaderMgr()
{
	SAFE_DELETE(mpMtrl);
}

void RShaderMgr::OnInvalidate()
{
	m_ShaderVec.clear();
	m_ShaderDeclVec.clear();
}

void RShaderMgr::OnRestore()
{
	Initialize();
}

bool RShaderMgr::Initialize()
{
	if (!RIsSupportVS()) return false;

	if (m_ShaderVec.size() == 0)
	{
		HRESULT hr;
		D3DPtr<IDirect3DVertexDeclaration9> VShaderDecl;

		D3DVERTEXELEMENT9 decl[] =
		{
			{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,		0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,	0 },
			{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES,	0 },
			{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
			{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		0 },

			D3DDECL_END()
		};

		if (FAILED(hr = RGetDevice()->CreateVertexDeclaration(decl, MakeWriteProxy(VShaderDecl))))
		{
			MLog("RShaderMgr::Initialize -- Failed to create vertex declaration\n");
			return false;
		}

		m_ShaderDeclVec.push_back(std::move(VShaderDecl));

		auto Shader = CreateVertexShader(skinData);
		if (!Shader)
		{
			assert(!"Shader Compile Error");
			mlog("Shader Compile Error");
			return false;
		}

		m_ShaderVec.push_back(std::move(Shader));

		if (m_ShaderVec.size() == 1)
		{
			init();
		}
	}

	return true;
}

LPDIRECT3DVERTEXSHADER9 RShaderMgr::getShader(int i_)
{
	if (i_ >= (int)m_ShaderVec.size())
	{
		assert(false);
		mlog(" Shader Critical Error \n ");
		return nullptr;
	}
	return m_ShaderVec[i_].get();
}

LPDIRECT3DVERTEXDECLARATION9 RShaderMgr::getShaderDecl(int i)
{
	if (i >= (int)m_ShaderDeclVec.size())
	{
		assert(false);
		mlog(" ShaderDecl Critical Error \n ");
		return nullptr;
	}
	return m_ShaderDeclVec[i].get();
}

void RShaderMgr::setMtrl(RMtrl* pmtrl_, float fVisAlpha_)
{
	if (!mbUsingShader) return;

	if (!pmtrl_) return;

	mpMtrl->m_diffuse.r = 0.5f;
	mpMtrl->m_diffuse.g = 0.5f;
	mpMtrl->m_diffuse.b = 0.5f;
	mpMtrl->m_diffuse.a = fVisAlpha_;

	mpMtrl->m_ambient.r = 0.35f;
	mpMtrl->m_ambient.g = 0.35f;
	mpMtrl->m_ambient.b = 0.35f;
	mpMtrl->m_ambient.a = 1.0f;

	mpMtrl->m_specular.r = 1.0f;
	mpMtrl->m_specular.g = 1.0f;
	mpMtrl->m_specular.b = 1.0f;
	pmtrl_->m_specular.a = 1.0f - fVisAlpha_;
}

void RShaderMgr::setMtrl(color_r32& rColor_, float fVisAlpha_)
{
	if (!mbUsingShader) return;

	mpMtrl->m_diffuse.r = rColor_.r;
	mpMtrl->m_diffuse.g = rColor_.g;
	mpMtrl->m_diffuse.b = rColor_.b;
	mpMtrl->m_diffuse.a = rColor_.a;

	mpMtrl->m_ambient.r = rColor_.r*0.2f;
	mpMtrl->m_ambient.g = rColor_.g*0.2f;
	mpMtrl->m_ambient.b = rColor_.b*0.2f;
	mpMtrl->m_ambient.a = 1.0f;

	mpMtrl->m_specular.r = 1.f;
	mpMtrl->m_specular.g = 1.f;
	mpMtrl->m_specular.b = 1.f;
	mpMtrl->m_specular.a = 1.f;
}

void RShaderMgr::setLight(int iLignt_, D3DLIGHT9* pLight_)
{
	if (!mbUsingShader) return;
	if (iLignt_ >= MAX_LIGHT) return;

	mLight[iLignt_].Ambient = pLight_->Ambient;
	mLight[iLignt_].Diffuse = pLight_->Diffuse;
	mLight[iLignt_].Specular = pLight_->Specular;
	mLight[iLignt_].Range = pLight_->Range;
	mLight[iLignt_].Attenuation0 = pLight_->Attenuation0;
	mLight[iLignt_].Attenuation1 = pLight_->Attenuation1;
	mLight[iLignt_].Attenuation2 = pLight_->Attenuation2;
	mLight[iLignt_].Position = pLight_->Position;
}

void RShaderMgr::setAmbient(u32 value_)
{
	if (!mbUsingShader) return;

	float global_ambient[4];
	global_ambient[3] = 0.f;
	global_ambient[0] = ((value_ & 0x00ff0000) >> 16) / (float)0xff;
	global_ambient[1] = ((value_ & 0x0000ff00) >> 8) / (float)0xff;
	global_ambient[2] = ((value_ & 0x000000ff)) / (float)0xff;

	RGetDevice()->SetVertexShaderConstantF(GLOBAL_AMBIENT, (float*)&global_ambient, 1);
}

void RShaderMgr::Update()
{
	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();

	// Update lights
	for (size_t i{}; i < 2; ++i)
	{
		auto SetShaderConstant = [&](UINT Register, auto& Val) {
			dev->SetVertexShaderConstantF(Register, reinterpret_cast<float*>(&Val), 1);
		};

		UINT Position[] = { LIGHT0_POSITION, LIGHT1_POSITION };
		UINT Ambient[] = { LIGHT0_AMBIENT, LIGHT1_AMBIENT };
		UINT Diffuse[] = { LIGHT0_DIFFUSE, LIGHT1_DIFFUSE };
		UINT Specular[] = { LIGHT0_SPECULAR, LIGHT1_SPECULAR };
		UINT Range[] = { LIGHT0_RANGE, LIGHT1_RANGE };
		UINT Attenuation[] = { LIGHT0_ATTENUATION, LIGHT1_ATTENUATION };
		UINT* Vals[] = { Position, Ambient, Diffuse, Specular, Range, Attenuation };

		if (mbLight[i])
		{
			SetShaderConstant(Position[i], mLight[i].Position);
			SetShaderConstant(Ambient[i], mLight[i].Ambient);
			SetShaderConstant(Diffuse[i], mLight[i].Diffuse);
			SetShaderConstant(Specular[i], mLight[i].Specular);
			SetShaderConstant(Range[i], mLight[i].Range);
			SetShaderConstant(Attenuation[i],
				v3{ mLight[i].Attenuation0, mLight[i].Attenuation1, mLight[i].Attenuation2 });
		}
		else
		{
			for (auto& Val : Vals)
				SetShaderConstant(Val[i], v4{ 0, 0, 0, 0 });
		}
	}
	
	// Update material
	dev->SetVertexShaderConstantF(MATERIAL_AMBIENT, (float*)&mpMtrl->m_ambient, 1);
	dev->SetVertexShaderConstantF(MATERIAL_DIFFUSE, (float*)&mpMtrl->m_diffuse, 1);
	dev->SetVertexShaderConstantF(MATERIAL_SPECULAR, (float*)&mpMtrl->m_specular, 1);
	dev->SetVertexShaderConstantF(MATERIAL_POWER, (float*)&mpMtrl->m_power, 1);
}

void RShaderMgr::init()
{
	LPDIRECT3DDEVICE9 dev = RealSpace2::RGetDevice();

	v4 constv{ 0, 0, 0, 0 };
	v4 constvatten{ 0.1f, 0.1f, 0.1f, 0.1f };

	float fConst[] = {
		1.0f, -1.0f, 0.5f, 255.f
	};

	dev->SetVertexShaderConstantF(CONSTANTS, fConst, 1);

	//shader constant register initialize
	dev->SetVertexShaderConstantF(LIGHT0_ATTENUATION, (float*)&constvatten, 1);
	dev->SetVertexShaderConstantF(LIGHT0_POSITION, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT0_AMBIENT, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT0_DIFFUSE, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT0_SPECULAR, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT0_RANGE, (float*)&constv, 1);

	dev->SetVertexShaderConstantF(LIGHT1_ATTENUATION, (float*)&constvatten, 1);
	dev->SetVertexShaderConstantF(LIGHT1_POSITION, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT1_AMBIENT, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT1_DIFFUSE, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT1_SPECULAR, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(LIGHT1_RANGE, (float*)&constv, 1);

	// update material
	dev->SetVertexShaderConstantF(MATERIAL_AMBIENT, (float*)&constv, 1);
	dev->SetVertexShaderConstantF(MATERIAL_DIFFUSE, (float*)&constv, 1);
	//dev->SetVertexShaderConstantF( MATERIAL_SPECULAR, (float*)&constv, 1 );
	dev->SetVertexShaderConstantF(MATERIAL_POWER, (float*)&constv, 1);
}

RShaderMgr* RShaderMgr::getShaderMgr()
{
	return &msInstance;
}

void RShaderMgr::LightEnable(int iLignt_, bool bEnable_)
{
	if (iLignt_ >= MAX_LIGHT) return;

	mbLight[iLignt_] = bEnable_;
}

void RShaderMgr::SetDisable()
{
	OnInvalidate();
	mbUsingShader = false;
}

bool RShaderMgr::SetEnable()
{
	if (Initialize())
	{
		mbUsingShader = true;
		return true;
	}
	else
	{
		mbUsingShader = false;
		return false;
	}
}