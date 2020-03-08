#include "stdafx.h"

#include "ZApplication.h"
#include "ZEffectBillboard.h"
#include "MZFileSystem.h"
#include "RTypes.h"

struct BillboardVertex {
	FLOAT	x, y, z;
	DWORD	color;
	FLOAT	tu, tv;
};

constexpr u32 BillboardFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

static LPDIRECT3DVERTEXBUFFER9 g_pVB;

bool CreateCommonRectVertexBuffer()
{
	static BillboardVertex Billboard[] = {
		{-1, -1, 0, 0xFFFFFFFF, 1, 0},
		{-1,  1, 0, 0xFFFFFFFF, 1, 1},
		{ 1,  1, 0, 0xFFFFFFFF, 0, 1},
		{ 1, -1, 0, 0xFFFFFFFF, 0, 0},
	};

	if (FAILED(RGetDevice()->CreateVertexBuffer(sizeof(Billboard), 0, BillboardFVF, D3DPOOL_MANAGED, &g_pVB, NULL)))
		return false;

	BYTE* pVertices;

	if(FAILED(g_pVB->Lock(0, sizeof(Billboard), (VOID**)&pVertices, 0))) return false;
	memcpy(pVertices, Billboard, sizeof(Billboard));
	g_pVB->Unlock();

	return true;
}

void RealeaseCommonRectVertexBuffer()
{
	SAFE_RELEASE(g_pVB);
}

static LPDIRECT3DVERTEXBUFFER9 GetCommonRectVertexBuffer()
{
	return g_pVB;
}

ZEffectBillboardSource::ZEffectBillboardSource(const char* szTextureFileName)
{
	m_pTex = RCreateBaseTexture(szTextureFileName);
}

ZEffectBillboardSource::~ZEffectBillboardSource(void)
{
	if(m_pTex) {
		RDestroyBaseTexture(m_pTex);
		m_pTex = NULL;
	}
}

bool ZEffectBillboardSource::Draw(rvector &Pos, rvector &Dir, rvector &Up, rvector &Scale, float fOpacity)
{
	if (m_pTex == NULL) return false;

	RealSpace2::rboundingbox bbox;

	rvector _scale = Scale/2;

	bbox.vmin = Pos-_scale;
	bbox.vmax = Pos+_scale;

	if (isInViewFrustum(bbox, RGetViewFrustum()) == false) {
  		return false;
	}

	// Transform
	rmatrix matTranslation, matScaling, matWorld;

	auto right = Normalized(CrossProduct(Up, Dir));
	auto up = Normalized(CrossProduct(right, Dir));

	rvector dir = RCameraDirection;

	rmatrix mat;
	GetIdentityMatrix(mat);
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=up.x;mat._22=up.y;mat._23=up.z;
	mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

	matTranslation = TranslationMatrix(Pos);
	matScaling = ScalingMatrix(Scale);

	matWorld = matScaling * mat;
	matWorld *= matTranslation;
	RSetTransform(D3DTS_WORLD, matWorld);

	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)((BYTE)(0xFF*fOpacity))<<24);

	RGetDevice()->SetStreamSource(0, GetCommonRectVertexBuffer(), 0, sizeof(BillboardVertex));
	RGetDevice()->SetFVF(BillboardFVF);
	RGetDevice()->SetTexture(0, m_pTex->GetTexture());
	RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	RGetDevice()->SetStreamSource( 0, NULL, 0, 0 );

	return true;
}

ZEffectBillboard::ZEffectBillboard(ZEffectBillboardSource* pEffectBillboardSource)
{
	m_Pos.x = m_Pos.y = m_Pos.z = 0;
	m_Scale.x = m_Scale.y = m_Scale.z = 1;
	m_fOpacity = 1;
	m_Normal.x = m_Normal.y = 0; m_Normal.z = 1;
	m_Up.x = m_Up.y = 1; m_Up.z = 0;

	m_pEffectBillboardSource = pEffectBillboardSource;
}

ZEffectBillboard::~ZEffectBillboard(void)
{
}

bool ZEffectBillboard::Draw(u64 nTime)
{
	if(m_pEffectBillboardSource!=NULL) {
		if(m_bRender) {
			m_bisRendered = m_pEffectBillboardSource->Draw(m_Pos, m_Normal, m_Up, m_Scale, m_fOpacity);
		}
		else m_bisRendered = false;
	}
	return true;
}



ZEffectBillboardDrawer::ZEffectBillboardDrawer(void)
{
	m_bCreate = false;
}

ZEffectBillboardDrawer::~ZEffectBillboardDrawer(void)
{
	m_bCreate = false;
}

void ZEffectBillboardDrawer::Create(void)
{
	if(m_bCreate) return;

	m_bCreate = true;
}

bool ZEffectBillboardDrawer::Draw(LPDIRECT3DTEXTURE9 pEffectBillboardTexture, rvector &Pos,
	rvector &Dir, rvector &Up, rvector &Scale, float fOpacity)
{
	if(isInViewFrustum( Pos, RGetViewFrustum())==false) {
		return false;
	}

	rmatrix matTranslation, matScaling, matWorld;

	auto right = Normalized(CrossProduct(Up, Dir));
	auto up = Normalized(CrossProduct(right, Dir));

	rvector dir = RCameraDirection;

	rmatrix mat;
	GetIdentityMatrix(mat);
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=up.x;mat._22=up.y;mat._23=up.z;
	mat._31=dir.x;mat._32=dir.y;mat._33=dir.z;

	matTranslation = TranslationMatrix(Pos);
	matScaling = ScalingMatrix(Scale);

	matWorld = matScaling * mat;
	matWorld *= matTranslation;
	RSetTransform(D3DTS_WORLD, matWorld);

	RGetDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

	RGetDevice()->SetStreamSource(0, GetCommonRectVertexBuffer(), 0, sizeof(BillboardVertex));
	RGetDevice()->SetFVF(BillboardFVF);
	RGetDevice()->SetTexture(0, pEffectBillboardTexture);
	RGetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	RGetDevice()->SetStreamSource( 0, NULL, 0, 0 );

	return true;
}

ZEffectBillboardDrawer	ZEffectBillboard2::m_EffectBillboardDrawer;

ZEffectBillboard2::ZEffectBillboard2(LPDIRECT3DTEXTURE9 pEffectBillboardTexture)
{
	m_Pos.x = m_Pos.y = m_Pos.z = 0;
	m_Scale.x = m_Scale.y = m_Scale.z = 1;
	m_fOpacity = 1;
	m_Normal.x = m_Normal.y = 0; m_Normal.z = 1;
	m_Up.x = m_Up.y = 1; m_Up.z = 0;

	m_pEffectBillboardTexture = pEffectBillboardTexture;

	if(m_EffectBillboardDrawer.IsCreated()==false) m_EffectBillboardDrawer.Create();
}

ZEffectBillboard2::~ZEffectBillboard2(void)
{
}

bool ZEffectBillboard2::Draw(u64 nTime)
{
	if(m_bRender)
		m_bisRendered = m_EffectBillboardDrawer.Draw(m_pEffectBillboardTexture, m_Pos, m_Normal, m_Up, m_Scale, m_fOpacity);
	else m_bisRendered = false;

	return true;
}

