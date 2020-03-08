#include "stdafx.h"
#include "ZApplication.h"
#include "ZMeshView.h"
#include "RealSpace2.h"
#include "RShaderMgr.h"
#include "MColorTable.h"
#include "mdebug.h"
#include "VertexTypes.h"

std::list<ZMeshView*> ZMeshView::msMeshViewList;

RVisualMesh* RTVisualMesh::GetVMesh(bool b)
{
	if(m_pVisualMesh->m_pMesh==NULL) {
		if( bInit == false ) {
			if(b) {
				mlog("RTVisualMesh::GetVMesh() -- bInit was false\n");
			}
		}
	}
	return m_pVisualMesh;
}

void ZMeshView::DrawTestScene()
{
	RSetProjection(PI_FLOAT/4, 1.0f, 1.0f, 1000.0f);
	RSetCamera(rvector(0.0f, 3.0f, -5.0f), rvector(0.0f, 0.0f, 0.0f), rvector(0.0f, 1.0f, 0.0f));

	RSetTransform(D3DTS_WORLD, GetIdentityMatrix());

	WorldSpaceColorVertex g_Vertices[] =
    {
        { -1.0f,-1.0f, 0.0f, 0xffff0000, },
        {  0.0f, 1.0f, 0.0f, 0xffffffff, },
        {  1.0f,-1.0f, 0.0f, 0xff0000ff, },
    };

	RGetDevice()->SetFVF(WorldSpaceColorFVF);
    RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, g_Vertices, sizeof(WorldSpaceColorVertex));
}

void ZMeshView::SetLight(rvector LPos)
{
	D3DLIGHT9 light;

	ZeroMemory( &light, sizeof(D3DLIGHT9) );

	light.Type = D3DLIGHT_POINT;

	light.Attenuation0 = 0.f;
	light.Attenuation1 = 0.005f;
	light.Attenuation2 = 0.f;

	light.Position = LPos;

	light.Ambient.r = 0.125f;
	light.Ambient.g = 0.125f;
	light.Ambient.b = 0.125f;

	light.Diffuse.r  = 0.175f;
	light.Diffuse.g  = 0.175f;
	light.Diffuse.b  = 0.175f;

	light.Specular.r = 1.f;
	light.Specular.g = 1.f;
	light.Specular.b = 1.f;

	light.Range      = 500.f;

	m_pTVisualMesh.GetVMesh(false)->SetLight(0, &light, false);

	light.Attenuation0	= 0.f;
	light.Attenuation1 = 0.005f;
	light.Attenuation2 = 0.f;

	light.Position = LPos;

	light.Ambient.r = 0.0f;
	light.Ambient.g = 0.0f;
	light.Ambient.b = 0.0f;

	light.Diffuse.r  = 0.0f;
	light.Diffuse.g  = 0.0f;
	light.Diffuse.b  = 0.0f;

	light.Specular.r = 1.f;
	light.Specular.g = 1.f;
	light.Specular.b = 1.f;

	light.Range       = 0.f;

	m_pTVisualMesh.GetVMesh(false)->SetLight(1,&light,true);
}

void ZMeshView::OnDraw(MDrawContext* pDC)
{
	MRECT r = GetClientRect();

	if (m_bLook) MButton::OnDraw(pDC);

	if(m_pTVisualMesh.GetVMesh(false)==NULL) 
		return;

	m_pTVisualMesh.GetVMesh(false)->SetVisibility((float)pDC->GetOpacity()/255.f);

	// From character drawing in zgame.cpp
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false );
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true );

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetTexture(0,NULL);
	pd3dDevice->SetTexture(1,NULL);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

	RGetDevice()->SetRenderState(D3DRS_CULLMODE  ,D3DCULL_NONE);

	RGetDevice()->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	RGetDevice()->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// Light
 	RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00cccccc );
	
	RGetShaderMgr()->setAmbient( 0x00ccccccc );

	// Z Buffer
	RGetDevice()->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1, 0);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	RSetWBuffer(true);
	RGetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	RSetTransform(D3DTS_VIEW, ViewMatrix(m_Eye, Normalized(m_At - m_Eye), m_Up));

	auto Proj = PerspectiveProjectionMatrix(r.w / float(r.h > 0 ? r.h : 1), PI_FLOAT / 4, 10, 1000);
	RSetTransform(D3DTS_PROJECTION, Proj);

	SetLight( rvector(50.f,200.f,-100.f) );

	rmatrix World;

	rvector Pos = rvector(0,0,0);
	rvector Dir = rvector(0,0,-1);
	rvector Up  = rvector(0,1,0);

	rmatrix m = RGetRotY(m_fCRot);
	Dir = Dir * m;

	Normalize(Dir);

	MakeWorldMatrix(&World, Pos, Dir, Up);

//#define _NPC_TEST

#ifndef _NPC_TEST

	m_pTVisualMesh.GetVMesh(false)->SetWorldMatrix(World);
	m_pTVisualMesh.GetVMesh(false)->Frame();
	m_pTVisualMesh.GetVMesh(false)->Render();

#else

	static RVisualMesh vmesh;

	ZGetNpcMeshMgr()->Load("goblinG");

//	if(vmesh.m_pMesh==NULL) 
	{
		RMesh* pMesh = ZGetNpcMeshMgr()->Get("goblinG");
		vmesh.Create( pMesh );
		vmesh.SetAnimation("idle");
	}

	vmesh.SetWorldMatrix(World);
	vmesh.Frame();
	vmesh.Render();

	ZGetNpcMeshMgr()->UnLoad("goblinG");

#endif
}

ZMeshView::ZMeshView(const char* szName, MWidget* pParent, MListener* pListener)
	: MButton(szName, pParent, pListener)
{
	m_fCRot = 0.f;
	m_fDist = 150.0f;
	m_fMaxDist = 300.0f;
	m_fMinDist = 50.0f;

	m_Eye = rvector(0.0f, 0.0f, -m_fDist);
	m_At = rvector(0.0f, 0.0f, 0.0f);
	m_Up = rvector(0.0f, 1.0f, 0.0f);

	m_bEnableRotate = false;
	m_bEnableZoom = false;
	m_bLook = false;

	SetEnableRotateZoom(true, true);

	msMeshViewList.push_back(this);
}

ZMeshView::~ZMeshView(void)
{
	for( list<ZMeshView*>::iterator iter = msMeshViewList.begin(); iter != msMeshViewList.end(); ++iter )
	{
		ZMeshView* p = *iter;
		if( p == this ) 
		{
			iter =  msMeshViewList.erase(iter);
			break;
		}
	}
}

void ZMeshView::SetEnableRotateZoom(bool bEnableRotate, bool bEnableZoom)
{
	m_bEnableRotate = bEnableRotate;
	m_bEnableZoom = bEnableZoom;
}

void ZMeshView::ZoomIn(float add_distance)
{
	if (m_bEnableZoom) 
	{
		m_fDist -= add_distance;
		if (m_fDist < m_fMinDist) m_fDist = m_fMinDist;
		else if (m_fDist > m_fMaxDist) m_fDist = m_fMaxDist;

		m_Eye.z = -m_fDist;
	}
}

void ZMeshView::ZoomOut(float add_distance)
{
	if (m_bEnableZoom) 
	{
		m_fDist += add_distance;
		if (m_fDist > m_fMaxDist) m_fDist = m_fMaxDist;
		else if (m_fDist < m_fMinDist) m_fDist = m_fMinDist;

		m_Eye.z = -m_fDist;
	}
}

void ZMeshView::RotateVertical(float add_degree)
{
	if (m_bEnableZoom) 
	{
		m_Eye.y -= add_degree;

		if      ( m_Eye.y > 200.0f)  m_Eye.y = 200.0f;
		else if ( m_Eye.y < 4.0f)    m_Eye.y = 4.0f;
	}
}

bool ZMeshView::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT r = GetInitialClientRect();

	static MPOINT st_LastPoint = pEvent->Pos;

	switch(pEvent->nMessage)
	{
	case MWM_MOUSEMOVE:
		{
			if(m_bLButtonDown) 
			{
				RotateLeft(1.5f * (st_LastPoint.x - pEvent->Pos.x));

				RotateVertical( 1.0f * (st_LastPoint.y - pEvent->Pos.y));
			}
		}
		break;
	case MWM_MOUSEWHEEL:
		{
			if(r.InPoint(pEvent->Pos)==false) return false;
			float fDist = min(max(-pEvent->nDelta, -8), 8);
			ZoomIn(fDist);
		}
		break;

	case MWM_LBUTTONDBLCLK:
		{
			m_bEnableRotate = (m_bEnableRotate == true) ? false : true;
		}
		break;
	}

	st_LastPoint = pEvent->Pos;

	return MButton::OnEvent(pEvent, pListener);
}