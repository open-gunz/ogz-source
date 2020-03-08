#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_NAMESPACE_REALSPACE2_BEGIN

void RMesh::SetMtrlUvAni_ON()
{
	if( m_pVisualMesh && m_pVisualMesh->m_bUVAni) {

		rmatrix mat;
		GetIdentityMatrix(mat);

		float add_t = GetGlobalTimeMS() / 1000.f;

		mat._31 = add_t * m_pVisualMesh->m_fUAniValue;
		mat._32 = add_t * m_pVisualMesh->m_fVAniValue;

		RGetDevice()->SetTransform( D3DTS_TEXTURE0, static_cast<const D3DMATRIX*>(mat));
		RGetDevice()->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 ); 
		RGetDevice()->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	}
}

void RMesh::SetMtrlUvAni_OFF()
{
	if( m_pVisualMesh && m_pVisualMesh->m_bUVAni) {//off

		RGetDevice()->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

	}
}


void RMesh::SetShaderDiffuseMap_OFF()
{
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void RMesh::SetShaderAlphaMap_OFF()
{
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RGetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );

	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	RGetDevice()->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	RGetDevice()->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	RGetDevice()->SetRenderState( D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(1.f,1.f,1.f,1.f));
}

void RMesh::SetShaderAdditiveMap_OFF()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dev->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
	dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);

	if(RGetFog())
		dev->SetRenderState( D3DRS_FOGENABLE, TRUE );
}

void RMesh::SetShaderNormalMap_OFF()
{

}

void RMesh::SetShaderAlphaTestMap_OFF()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
	dev->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
}

void RMesh::SetShaderDiffuseMap(RMtrl* pMtrl,DWORD color)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
	dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	if(color == 0x0000ff00) {

		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		if( m_LitVertexModel || GetToolMesh()) 
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		else {
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
		}
	}
	else {

		dev->SetRenderState( D3DRS_TEXTUREFACTOR, color);

		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		dev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		dev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

		if( m_LitVertexModel || GetToolMesh()) 
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		else {
			dev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
		}
	}
}

void RMesh::SetShaderAlphaTestMap(int value,float fVis)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	BYTE _ref = (BYTE)value;

	if(fVis == 1.f) {
		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}
	else {

		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	dev->SetRenderState( D3DRS_ALPHAREF, _ref );

	dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

	if(m_isNPCMesh || m_isCharacterMesh && !GetToolMesh())
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
	else
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RMesh::SetShaderAlphaMap()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	dev->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	dev->SetRenderState( D3DRS_ALPHAREF, 0x04 );
	dev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

	if(m_isNPCMesh || m_isCharacterMesh && !GetToolMesh())
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
	else
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RMesh::SetShaderAdditiveMap()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
	dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);

	dev->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
	dev->SetRenderState( D3DRS_ALPHAREF,         0x04 );
	dev->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

	if(RGetFog())
		dev->SetRenderState( D3DRS_FOGENABLE, FALSE );

	dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
		
}

void RMesh::SetShaderNormalMap()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);

	if(m_isNPCMesh || m_isCharacterMesh && !GetToolMesh())
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
	else
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

int RMesh::GetCharacterMtrlMode(RMtrl* pMtrl,float vis_alpha)
{
	if(pMtrl) {

		vis_alpha = min(vis_alpha,m_fVis);

		if( pMtrl->m_bDiffuseMap ) {
			return -1;
		} else if( pMtrl->m_bAlphaMap || vis_alpha != 1.f) {
			return eRRenderNode_Alpha;
		} else if( pMtrl->m_bAdditive ) {
			return eRRenderNode_Add;
		} else {
			return -1;
		}
	}
	return -1;
}

void RMesh::SetCharacterMtrl_ON(RMtrl* pMtrl,RMeshNode* pMNode,float vis_alpha,DWORD color)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	D3DCOLORVALUE _color;

	_color.r = 0.6f;
	_color.g = 0.6f;
	_color.b = 0.6f;
	_color.a = 1;

	if( m_pVisualMesh && !pMNode->m_bNpcWeaponMeshNode ) { 
		_color = m_pVisualMesh->m_NPCBlendColor; 
	}

	pMNode->SetMtrl(pMtrl,min(GetMeshNodeVis(pMNode),m_fVis) ,m_isNPCMesh ,_color);

	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);

	if(pMtrl) {

		if(pMtrl->m_bAlphaTestMap && m_is_map_object)
			m_LitVertexModel = true;

		vis_alpha = min(vis_alpha,m_fVis);

		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		if(m_LitVertexModel) {
			DWORD _c  = D3DCOLOR_COLORVALUE(1.f,1.f,1.f,vis_alpha);
			dev->SetRenderState( D3DRS_TEXTUREFACTOR, _c);
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
		}
		else {
			dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		}

		if( m_LitVertexModel || GetToolMesh() )
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		else
			dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );

		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);

		if(pMtrl->m_bTwoSided) {
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		}
		
		if( pMtrl->m_bDiffuseMap ) {
			SetShaderDiffuseMap(pMtrl,color);
		}

		if( pMtrl->m_bAlphaMap || vis_alpha != 1.f) {
			if(!pMtrl->m_bAlphaTestMap)
				SetShaderAlphaMap();
		}

		if( pMtrl->m_bAdditive ) {
			SetShaderAdditiveMap();
		}

		if( pMtrl->m_bAlphaTestMap ) {
			SetShaderAlphaTestMap( pMtrl->m_nAlphaTestValue ,vis_alpha );
		}
	}

	if( GetTextureRenderOnOff() && pMtrl )
		dev->SetTexture( 0, pMtrl->GetTexture());
	else
		dev->SetTexture( 0, NULL);

	if( m_nSpRenderMode==1 )
	{
		dev->SetTexture( 0, NULL);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO); 
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
	else if( m_nSpRenderMode==2 )
	{
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL); 
	}

	SkyBoxMtrlOn();
}

void RMesh::SetCharacterMtrl_OFF(RMtrl* pMtrl,float vis_alpha)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	SkyBoxMtrlOff();

	if(pMtrl) {

		vis_alpha = min(vis_alpha,m_fVis);

		if( pMtrl->m_bAdditive ) {
			SetShaderAdditiveMap_OFF();
		}

		if(pMtrl->m_bTwoSided) {
			dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}

		if( pMtrl->m_bAlphaTestMap ) {
			SetShaderAlphaTestMap_OFF();
		}
	}

	if( m_nSpRenderMode==1 )
	{
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); 
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); 
	}
	else if( m_nSpRenderMode==2 )
	{
		dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}

	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState( D3DRS_ALPHATESTENABLE , FALSE);

	dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	dev->SetRenderState( D3DRS_TEXTUREFACTOR, 0xffffffff);

}

rmatrix tmpWrd,tmpView;

void RMesh::SkyBoxMtrlOn()
{
	if( mbSkyBox ) 
	{
		LPDIRECT3DDEVICE9 dev = RGetDevice();

		dev->SetRenderState( D3DRS_LIGHTING, FALSE	 );
		RSetWBuffer( FALSE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		dev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		dev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	}
}

void RMesh::SkyBoxMtrlOff()
{
	if( mbSkyBox )
	{
		LPDIRECT3DDEVICE9 dev = RGetDevice();

		RSetWBuffer( TRUE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	}
}

_NAMESPACE_REALSPACE2_END

#undef __BP
#undef __EP