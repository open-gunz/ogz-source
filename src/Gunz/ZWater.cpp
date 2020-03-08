#include "stdafx.h"
#include "ZApplication.h"
#include "ZWater.h"
#include "ZEffectManager.h"
#include "ZSoundEngine.h"
#include "MDebug.h"
#include "ZConfiguration.h"
#include "RTypes.h"
#include "ZSoundEngine.h"
#include "ZEffectManager.h"
#include "RBspObject.h"

struct WaterVertex
{
	rvector p, n;
	float tu1, tv1;
	float tu2, tv2;
};

#define MAXNUM_WATER_MESH_VERTEX	2000
#define WATERVERTEX_TYPE	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
#define WATERTEXTURE_SIZE	512
#define WAVE_PITCH			50
#define WATER_UPDATE_INTERVAL 30
const char* waterHitSoundName = "fx_bullethit_mt_wat";

LPDIRECT3DDEVICE9		g_pDevice = g_pDevice;
LPDIRECT3DSURFACE9		g_pSufBackBuffer = 0;
LPDIRECT3DSURFACE9		g_pSufDepthBuffer = 0;
LPDIRECT3DSURFACE9		g_pSufReflection = 0;
LPDIRECT3DSURFACE9		g_pSufRefDepthBuffer = 0;
LPDIRECT3DTEXTURE9		g_pTexReflection = 0;
LPDIRECT3DVERTEXBUFFER9	g_pVBForWaterMesh = 0;
WaterVertex				g_waterVertexBuffer[MAXNUM_WATER_MESH_VERTEX];
RTLVertex				g_underWaterVertexBuffer[4];

ZWaterList::ZWaterList()
{
	OnRestore();
	m_dwTime = 0;

	constexpr auto TEXOFFSET = 0.01f;

	g_underWaterVertexBuffer[0].p.x = 0.0f - TEXOFFSET;
	g_underWaterVertexBuffer[0].p.y = 0.0f - TEXOFFSET;
	g_underWaterVertexBuffer[0].p.z = 0.0f;
	g_underWaterVertexBuffer[0].p.w = 1.0f;
	g_underWaterVertexBuffer[0].color = 0x33444444;
	g_underWaterVertexBuffer[0].tu = 0.0f;
	g_underWaterVertexBuffer[0].tv = 0.0f;
	g_underWaterVertexBuffer[1].p.x = 0.0f - TEXOFFSET;
	g_underWaterVertexBuffer[1].p.y = RGetScreenHeight() + TEXOFFSET;
	g_underWaterVertexBuffer[1].p.z = 0.0f;
	g_underWaterVertexBuffer[1].p.w = 1.0f;
	g_underWaterVertexBuffer[1].color = 0x33444444;
	g_underWaterVertexBuffer[1].tu = 0.0f;
	g_underWaterVertexBuffer[1].tv = 1.0f;
	g_underWaterVertexBuffer[2].p.x = RGetScreenWidth() + TEXOFFSET;
	g_underWaterVertexBuffer[2].p.y = RGetScreenHeight() + TEXOFFSET;
	g_underWaterVertexBuffer[2].p.z = 0.0f;
	g_underWaterVertexBuffer[2].p.w = 1.0f;
	g_underWaterVertexBuffer[2].color = 0x33444444;
	g_underWaterVertexBuffer[2].tu = 1.0f;
	g_underWaterVertexBuffer[2].tv = 1.0f;
	g_underWaterVertexBuffer[3].p.x = RGetScreenWidth() + TEXOFFSET;
	g_underWaterVertexBuffer[3].p.y = 0.0f - TEXOFFSET;
	g_underWaterVertexBuffer[3].p.z = 0.0f;
	g_underWaterVertexBuffer[3].p.w = 1.0f;
	g_underWaterVertexBuffer[3].color = 0x33444444;
	g_underWaterVertexBuffer[3].tu = 1.0f;
	g_underWaterVertexBuffer[3].tv = 0.0f;
}

ZWaterList::~ZWaterList()
{
	Clear();
	g_pDevice = NULL;
	SAFE_RELEASE( g_pVBForWaterMesh );
	SetSurface( false );
}

void ZWaterList::Clear()
{
	for( iterator iter = begin(); iter != end(); )
	{
		SAFE_DELETE(*iter);
		iter = erase(iter);
	}
}

void ZWaterList::Update()
{
	DWORD currentTime = GetGlobalTimeMS();
	if( (currentTime - m_dwTime) < WATER_UPDATE_INTERVAL ) return;
	m_dwTime = currentTime;	

	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZWater* pWater = *iter;
		if(pWater!=NULL) pWater->Update();
	}
}

void ZWaterList::Render()
{
	PreRender();
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		ZWater* pWater = *iter;
		if(pWater!=NULL) pWater->Render();
	}
	PostRender();
}

void ZWaterList::OnInvalidate()
{
	g_pDevice = NULL;
	SetSurface( false );
	SAFE_RELEASE(g_pVBForWaterMesh);
}

void ZWaterList::OnRestore()
{
	g_pDevice = RGetDevice();
	_ASSERT( g_pDevice != NULL );

	if( g_pVBForWaterMesh==NULL) {
		if(FAILED( g_pDevice->CreateVertexBuffer( MAXNUM_WATER_MESH_VERTEX * sizeof(WaterVertex), 
							D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, WATERVERTEX_TYPE, D3DPOOL_DEFAULT, &g_pVBForWaterMesh ,NULL)))
		{
			mlog( "Fail to Create Vertex Buffer for Water Mesh\n" );
			g_pVBForWaterMesh = 0;
		}
	}

	Z_VIDEO_REFLECTION = SetSurface( Z_VIDEO_REFLECTION );
}

bool ZWaterList::CheckSpearing(const rvector& o, const rvector& e,
	int iPower, float fArea, bool bPlaySound)
{	
	ZWater* pWater;
	rvector pos;
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		pWater	= *iter;
		if( pWater != NULL ) 
		{
			if( pWater->CheckSpearing( o, e, iPower, fArea, &pos ) )
			{
				ZGetSoundEngine()->PlaySound( (char*)waterHitSoundName, pos, false, false, 0 );

				float p		= iPower * 0.004f;
				pos.z += 0.1;
				ZGetEffectManager()->AddWaterSplashEffect( pos, rvector(p,p,p));
				return true;
			}
		}
	}
	return false;
}

bool ZWaterList::Pick( rvector& o, rvector& d, rvector* pPos )
{
	ZWater* pWater;
	for( iterator iter = begin(); iter != end(); ++iter )
	{
		pWater	= *iter;
		if( pWater != NULL ) 
		{
			if( pWater->Pick( o, d, pPos ) )
				return true;				
		}
	}
	return false;
}

ZWater* ZWaterList::Get( int iIndex )
{
	if( 0 > iIndex || size() <= (unsigned int)iIndex ) return NULL;
	iterator iter = begin();
	ZWater* pWater = *iter;
	pWater += iIndex;
	return pWater;
}

void ZWaterList::PreRender()
{
}

void ZWaterList::PostRender()
{
	g_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	g_pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	g_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0x00000000 );
	g_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	g_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
 	g_pDevice->SetRenderState( D3DRS_ALPHAFUNC,	D3DCMP_ALWAYS );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	 D3DTOP_DISABLE );
	g_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE );
}

bool ZWaterList::SetSurface(bool b)
{
	if(b)
	{
		RPIXELFORMAT pixelFormat = RGetPixelFormat();

		if(g_pTexReflection==NULL) {
			if(FAILED(RGetDevice()->CreateTexture(WATERTEXTURE_SIZE, WATERTEXTURE_SIZE,
				1, D3DUSAGE_RENDERTARGET, pixelFormat, D3DPOOL_DEFAULT, &g_pTexReflection, nullptr)))
			{
				if (FAILED(RGetDevice()->CreateTexture(WATERTEXTURE_SIZE, WATERTEXTURE_SIZE,
					1, D3DUSAGE_DYNAMIC | D3DUSAGE_RENDERTARGET, pixelFormat, D3DPOOL_DEFAULT,
					&g_pTexReflection, nullptr)))
				{
					mlog("Fail to Create Reflection Texture for Water Mesh\n");
					g_pTexReflection = 0;
					return false;
				}
			}
		}
		if(g_pSufReflection==NULL) {
			if(FAILED(g_pTexReflection->GetSurfaceLevel( 0, &g_pSufReflection )))
			{
				mlog( "Fail to Get Reflection Surface for Water Mesh\n" );
				g_pTexReflection = 0;
				g_pSufReflection = 0;
				return false;
			}
		}
        D3DSURFACE_DESC sufdesc;
		g_pSufReflection->GetDesc(&sufdesc);

		if(g_pSufRefDepthBuffer==NULL) {
			if(FAILED(g_pDevice->CreateDepthStencilSurface(sufdesc.Width, sufdesc.Height, D3DFMT_D16, D3DMULTISAMPLE_NONE,0,TRUE, &g_pSufRefDepthBuffer ,NULL)))
			{
				mlog( "Fail to Create Reflection Depth Buffer for Water Mesh\n" );
				g_pTexReflection = 0;
				g_pSufReflection = 0;
				g_pSufRefDepthBuffer = 0;
				return false;
			}
		}
		return true;
	}
	else
	{
		SAFE_RELEASE(g_pSufBackBuffer);
		SAFE_RELEASE(g_pSufDepthBuffer);
		SAFE_RELEASE(g_pSufReflection);
		SAFE_RELEASE(g_pSufRefDepthBuffer);
		SAFE_RELEASE(g_pTexReflection);
	}
	return false;
}

// ZWater
ZWater::ZWater()
{
	m_pIndexBuffer = 0;
	m_pTexture = 0;
	m_pVerts = 0;
	m_pFaces = 0;
	m_nVerts = 0;
	m_nFaces = 0;
	m_nWaterType = 0;
	m_isRender = true;
}

ZWater::~ZWater()
{
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_DELETE_ARRAY(m_pVerts);
	SAFE_DELETE_ARRAY(m_pFaces);
}

bool ZWater::SetMesh( RMeshNode* meshNode )
{
	// check once
	SAFE_RELEASE(m_pIndexBuffer);
	_ASSERT( meshNode != NULL );
	if( meshNode == NULL ) return false;
	
	// assign
	m_nVerts = meshNode->m_point_num;
	m_nFaces = meshNode->m_face_num;	

	// set matrix
	rvector offset{ meshNode->m_mat_base(0, 0), meshNode->m_mat_base(0, 1), meshNode->m_mat_base(0, 2) };
	MakeWorldMatrix(&m_worldMat, offset, rvector(0,-1,0), rvector(0,0,1));
	m_worldMat = meshNode->m_mat_result* m_worldMat;

	// transform world space
	m_pVerts = new rvector[m_nVerts];
	for( int i = 0 ; i < m_nVerts; ++i )
	{
		m_pVerts[i] = Transform(meshNode->m_point_list[i], m_worldMat);
	}
	m_pFaces = new RFaceInfo[m_nFaces];
	for( int i = 0;i <m_nFaces;++i)
	{
		m_pFaces[i] = meshNode->m_face_list[i];
	}

	// set camera reflection z pos
	m_fbaseZpos = m_pVerts[0].z;	

	// set Bounding Box..
	m_BoundingBox.minx	= m_BoundingBox.miny	= m_BoundingBox.minz	= 9999999;
	m_BoundingBox.maxx	= m_BoundingBox.maxy	= m_BoundingBox.maxz	= -9999999;
	for(int i = 0; i < m_nVerts; ++i)
	{
		m_BoundingBox.minx	= min( m_BoundingBox.minx, m_pVerts[i].x );
		m_BoundingBox.miny	= min( m_BoundingBox.miny, m_pVerts[i].y );
		m_BoundingBox.minz	= min( m_BoundingBox.minz, m_pVerts[i].z - WAVE_PITCH );

		m_BoundingBox.maxx	= max( m_BoundingBox.maxx, m_pVerts[i].x );
		m_BoundingBox.maxy	= max( m_BoundingBox.maxy, m_pVerts[i].y );
		m_BoundingBox.maxz	= max( m_BoundingBox.maxz, m_pVerts[i].z + WAVE_PITCH );
	}

	// set Texture
	RMtrlMgr* pMtrlMgr = &meshNode->m_pParentMesh->m_mtrl_list_ex;
	if(pMtrlMgr != NULL ) 
	{
		RMtrl* pMtrl=NULL;
		pMtrl = pMtrlMgr->Get_s( meshNode->m_mtrl_id, -1 );
		m_pTexture = pMtrl->m_pTexture;

		// set Material
		memset(&m_mtrl,0, sizeof(D3DMATERIAL9));
		m_mtrl.Ambient = static_cast<D3DCOLORVALUE>(pMtrl->m_ambient);
		m_mtrl.Diffuse = static_cast<D3DCOLORVALUE>(pMtrl->m_diffuse);
		m_mtrl.Diffuse.a = 0.5;	m_mtrl.Power = 0;
		m_mtrl.Specular.a = 0;	m_mtrl.Specular.r = 0;	m_mtrl.Specular.g = 0;	m_mtrl.Specular.b = 0;
	}	

	// set index buffer
	WORD* indexList = new WORD[m_nFaces*3];
	for(int i=0;i<m_nFaces;++i)
	{
		for(int j=0;j<3;++j)
		{
			indexList[3*i+j] = meshNode->m_face_list[i].m_point_index[j];
		}
	}
	if(FAILED( g_pDevice->CreateIndexBuffer( m_nFaces * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexBuffer ,NULL)))
	{
		mlog( "Fail to Create Index Buffer \n" );
		return false;
	}
	VOID * pIndexes;
	if(FAILED(m_pIndexBuffer->Lock(0, m_nFaces * 3 * sizeof(WORD), (VOID**)&pIndexes, 0 )))
	{
		mlog( "Fail to Lock Index Buffer \n" );
		return false;
	}
	memcpy( pIndexes, indexList, m_nFaces * 3 * sizeof(WORD));
	if(FAILED( m_pIndexBuffer->Unlock() ))
	{
		mlog( "Fail to UnLock Index Buffer \n" );
		return false;
	}
	SAFE_DELETE_ARRAY(indexList);

	return true;
}

bool ZWater::RenderReflectionSurface()
{
	if(g_pSufReflection == NULL || g_pSufRefDepthBuffer == NULL ) return false;

	rmatrix viewOrg = RGetTransform(D3DTS_VIEW);

	// Set Camera Position.. 

	rvector cameraPos = RCameraPosition;
	cameraPos.z = m_fbaseZpos + (m_fbaseZpos-cameraPos.z);
	rvector cameraDir = RCameraDirection;
	cameraDir.z = -cameraDir.z;
	rvector cameraAt = cameraPos + cameraDir;
	rvector cameraUp = RCameraUp;
	cameraUp.z = -cameraUp.z;


	auto viewMat = ViewMatrix(cameraPos, cameraDir, cameraUp);
	RSetTransform(D3DTS_VIEW, viewMat);

	// Redirection Render Target
 	g_pDevice->GetRenderTarget( 0 , &g_pSufBackBuffer );
	g_pDevice->GetDepthStencilSurface( &g_pSufDepthBuffer );

	g_pDevice->SetRenderTarget( 0,g_pSufReflection );
	g_pDevice->SetDepthStencilSurface(g_pSufRefDepthBuffer);

	g_pDevice->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000,1.0f,0.0f);

	// pre render
	rmatrix proj = RGetTransform(D3DTS_PROJECTION);
	rmatrix transform = viewMat * proj;

	for( int i=0; i<m_nVerts; ++i )
	{
 		rvector temp = rvector(m_pVerts[i].x,m_pVerts[i].y, m_fbaseZpos) * transform;
		g_waterVertexBuffer[i].tu2 =        ( temp.x + 1.0f ) * 0.5f ;
		g_waterVertexBuffer[i].tv2 = 1.0f - ( temp.y + 1.0f ) * 0.5f ;
		
	}

	// render
	if(RIsAvailUserClipPlane())
	{
		g_pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, TRUE );
		rplane p = rplane(0,0,1,-m_fbaseZpos+10.0f);
		g_pDevice->SetClipPlane(0,(float*)p);
	}

	ZGetGame()->GetWorld()->GetBsp()->Draw();
	ZGetGame()->GetWorld()->GetBsp()->DrawObjects();

	if(RIsAvailUserClipPlane())
	{
		ZGetEffectManager()->Draw(GetGlobalTimeMS());
		ZGetGame()->GetWorld()->GetFlags()->Draw();
	}

	if(RIsAvailUserClipPlane())
	{
		g_pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE );
	}

	// Restore Render Target
	g_pDevice->SetRenderTarget( 0,g_pSufBackBuffer );
	g_pDevice->SetDepthStencilSurface(g_pSufDepthBuffer);

	RSetTransform(D3DTS_VIEW, viewOrg);
	SAFE_RELEASE(g_pSufBackBuffer);
	SAFE_RELEASE(g_pSufDepthBuffer);
	
	return true;
}

bool ZWater::RenderUnderWater()
{	
 	g_underWaterVertexBuffer[0].tu += 0.001f;
	g_underWaterVertexBuffer[1].p.y = RGetScreenHeight();
	g_underWaterVertexBuffer[1].tu += 0.001f;
	g_underWaterVertexBuffer[2].p.x = RGetScreenWidth();
	g_underWaterVertexBuffer[2].p.y = RGetScreenHeight();
	g_underWaterVertexBuffer[2].tu += 0.001f;
	g_underWaterVertexBuffer[3].p.x = RGetScreenWidth();
	g_underWaterVertexBuffer[3].tu += 0.001f;

	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
 	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	g_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0x66000000 );
	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
	g_pDevice->SetFVF( RTLVertexType );
	g_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, g_underWaterVertexBuffer, sizeof(RTLVertex) );

	return true;
}

RTLVertex m_buffer[4];//test
#define CORRECTION	0.01f
int m_w = 128;
int m_h = 128;

void MakeBuffer()
{

#define S_X 20.f
#define S_Y 200.f
#define S_W	( S_X + m_w )
#define S_H	( S_Y + m_h )

	// left top
	m_buffer[0].p		= { S_X - CORRECTION, S_Y - CORRECTION, 0, 1.0f };
	m_buffer[0].color	= 0xffffffff;
	m_buffer[0].tu		= 0.0f;
	m_buffer[0].tv		= 0.0f;

	// right top
	m_buffer[1].p		= { S_W + CORRECTION, S_Y - CORRECTION, 0, 1.0f };
	m_buffer[1].color	= 0xffffffff;
	m_buffer[1].tu		= 1.0f;
	m_buffer[1].tv		= 0.0f;

	// right bottom
	m_buffer[2].p		= { S_W + CORRECTION, S_H + CORRECTION, 0, 1.0f };
	m_buffer[2].color	= 0xffffffff;
	m_buffer[2].tu		= 1.0f;
	m_buffer[2].tv		= 1.0f;

	// left bottom
	m_buffer[3].p		= { S_X - CORRECTION, S_H + CORRECTION, 0, 1.0f };
	m_buffer[3].color	= 0xffffffff;
	m_buffer[3].tu		= 0.0f;
	m_buffer[3].tv		= 1.0f;

}

void RenderBuffer()
{
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	RGetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	RGetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	if( g_pTexReflection != 0 )
	{
//		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
//		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		RGetDevice()->SetTexture( 0, g_pTexReflection );
	}
	else
	{
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		RGetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	}

	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );

	float alpha	= 1.f;//max(min( ((mfPower - elapsedTime)/CLEAR_TIME_DURATION), 1.0f ), 0.0f);

	RGetDevice()->SetRenderState( D3DRS_TEXTUREFACTOR,(DWORD)((BYTE)( 0xFF * alpha ))<<24 );

	RGetDevice()->SetFVF( RTLVertexType );
//	DWORD dw;
//	RGetDevice()->GetRenderState( D3DRS_ZENABLE, &dw );
//	RGetDevice()->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
	RGetDevice()->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, m_buffer, sizeof(RTLVertex) );
//	RGetDevice()->SetRenderState( D3DRS_ZENABLE, dw );
}

void ZWater::Render()
{
	if(!m_isRender)
		return;

	//if(!isInViewFrustum(&m_BoundingBox, RGetViewFrustum()))	return;
	if(g_pGame==NULL || !ZGetGame()->GetWorld()->GetBsp()->IsVisible(m_BoundingBox)) 
		return;

	//반사 그리기
  	if(Z_VIDEO_REFLECTION && RCameraPosition.z > m_fbaseZpos)
	{
		if(!RenderReflectionSurface())
		{
			mlog("Fail to Render Reflection Surface..\n");
		}
	}

//	RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );

  	g_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	g_pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, (DWORD)0x77000000 );
  	g_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

	g_pDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	g_pDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );
//	g_pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
//	g_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);

	g_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
 	g_pDevice->SetRenderState( D3DRS_ALPHAFUNC,	D3DCMP_ALWAYS );
  
	if( Z_VIDEO_REFLECTION && RCameraPosition.z > m_fbaseZpos )
	{ 
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
 		g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
		g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
 		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
 		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADDSMOOTH );

		g_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
 		g_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

 		g_pDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		g_pDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	}
	else
	{
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
		g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	 D3DTOP_DISABLE );
		g_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE );
	}
	
	// copy vertices
	for(int i =0; i<m_nVerts; ++i) 
	{
		g_waterVertexBuffer[i].p = m_pVerts[i];
	}

	for(int i = 0; i<m_nFaces; ++i) 
	{
		for( int j=0; j<3; ++j ) 
		{
			int index = m_pFaces[i].m_point_index[j];
			g_waterVertexBuffer[index].tu1 = m_pFaces[i].m_point_tex[j].x;
 			g_waterVertexBuffer[index].tv1 = m_pFaces[i].m_point_tex[j].y;
		}
	}

	void* pVertices;

	if(FAILED( g_pVBForWaterMesh->Lock(0, m_nVerts*sizeof( WaterVertex ),(VOID**)&pVertices,0)))
	{
		mlog("Fail to Lock vertex buffer for water mesh\n");
		return;
	}

	memcpy( pVertices, g_waterVertexBuffer, m_nVerts* sizeof( WaterVertex ));

	if(FAILED(g_pVBForWaterMesh->Unlock()))
	{
		mlog("Fail to Unlock vertex buffer for water mesh\n");
		return;
	}

	// set texture
	if( m_pTexture != NULL )
 		g_pDevice->SetTexture( 0, m_pTexture->GetTexture() );


	g_pDevice->SetTexture(1, g_pTexReflection );


	// set material
	g_pDevice->SetMaterial(&m_mtrl);

	// set transform
	rmatrix mat;
	GetIdentityMatrix(mat);
	RSetTransform(D3DTS_WORLD, mat);

	// render
	g_pDevice->SetStreamSource( 0, g_pVBForWaterMesh, 0,sizeof(WaterVertex) );
	g_pDevice->SetIndices( m_pIndexBuffer );
	g_pDevice->SetFVF( WATERVERTEX_TYPE );
	g_pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0, 0, m_nVerts, 0, m_nFaces);
}

void ZWater::Update()
{
	if (!isInViewFrustum(m_BoundingBox, RGetViewFrustum())) return;

	if(m_nWaterType != WaterType2)
   		Ripple( (rvector)m_BoundingBox.m[0], 5, 0.01f );
}

void ZWater::OnInvalidate()
{
}

void ZWater::OnRestore()
{
}

bool ZWater::CheckSpearing(const rvector& o, const rvector& e, int iPower, float fArea, rvector* pPos )
{
    // test line vs AABB
	rvector dir = Normalized(e - o);

	return Pick( o, dir, pPos );
}

bool ZWater::Pick(const rvector& o, const rvector& d, rvector* pPos )
{
	if (!IntersectLineSegmentAABB(o, d, m_BoundingBox))
		return false;
	
	// line vs triangle test 
	int index[3], sIndex = -1;
	rvector uvt;
	rvector* v[3];

	for( int i = 0 ; i < m_nFaces; ++i )
	{
		for( int j = 0 ; j < 3; ++j )
		{
			index[j]= m_pFaces[i].m_point_index[j];
			v[j]	= &m_pVerts[index[j]];
		}

		if (IntersectTriangle(*v[0], *v[1], *v[2], o, d, nullptr, &uvt.x, &uvt.y))
		{
			*pPos = *v[0] + uvt.x * (*v[1] - *v[0]) + uvt.y * (*v[2] - *v[0]);
			return true;
		}
	}	
	return false;
}

void ZWater::Ripple(const rvector& pos, int iAmplitude, float fFrequency)
{
	DWORD dwTime = GetGlobalTimeMS();
	for( int i = 0; i <m_nVerts; ++i )
	{
		float fDist = Magnitude(pos - m_pVerts[i]);
 		m_pVerts[i].z = m_fbaseZpos + iAmplitude*(float)sin(fDist*fFrequency +(dwTime%6280)*0.001f);
	}
}