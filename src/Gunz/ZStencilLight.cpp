#include "stdafx.h"
#include "ZGame.h"
#include "ZStencilLight.h"
#include "RealSpace2.h"
#include "RMeshUtil.h"
#include "RBspObject.h"

#define CORRECTION		0.01f
#define SLSEGMENT		8
#define MAXLSID			60000
#define SLNITER			8
#define SLBASERADIUS	150

using namespace RealSpace2;

bool bInverse = false;

ZStencilLight* ZGetStencilLight() { return ZStencilLight::GetInstance(); }

ZStencilLight::ZStencilLight()
{
	m_id = 1;
	m_pTex = NULL;
	Mesh = CreateSphere(1, SLSEGMENT, SLSEGMENT);
}

ZStencilLight::~ZStencilLight()
{
	Destroy();
}

void ZStencilLight::Destroy()
{
	if(m_pTex)
	{
		RDestroyBaseTexture(m_pTex);
		m_pTex = NULL;
	}

	for (map<int, LightSource*>::iterator iter = m_LightSource.begin(); iter != m_LightSource.end(); )
	{
		LightSource* pLS = iter->second;
		SAFE_DELETE(pLS);
		iter = m_LightSource.erase(iter);
	}
	LightSource::Release();

	Mesh.Destroy();
}


void ZStencilLight::PreRender()
{
	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();

	pd3dDevice->SetFVF( LIGHT_BSP_FVF );

	pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0xffffffff );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

	pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0);
	pd3dDevice->LightEnable( 0, FALSE );
	pd3dDevice->LightEnable( 1, FALSE );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true );
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	RSetTransform(D3DTS_WORLD, IdentityMatrix());
}

void ZStencilLight::RenderStencil()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);

	// update world matrix
	rmatrix world;
	GetIdentityMatrix(world);
	world._11 = m_Radius;
	world._22 = m_Radius;
	world._33 = m_Radius;
	world._41 = m_Position.x;
	world._42 = m_Position.y;
	world._43 = m_Position.z;
	RSetTransform(D3DTS_WORLD, world);

	// render stencil
	dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	Mesh.Draw();

	dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Mesh.Draw();
}

void ZStencilLight::RenderStencil(const rvector& p, float raidus)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
 
	dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);

	// update world matrix
	rmatrix world;
	GetIdentityMatrix(world);
	world._11 = raidus;
	world._22 = raidus;
	world._33 = raidus;
	world._41 = p.x;
	world._42 = p.y;
	world._43 = p.z;
	RSetTransform(D3DTS_WORLD, world);

	if(bInverse)
	{
		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		Mesh.Draw();

		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		Mesh.Draw();
	}
	else
	{
		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		Mesh.Draw();

		dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		Mesh.Draw();
	}	
	bInverse = false;
}
 
void ZStencilLight::RenderLight()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	static bool init = false;
	if(!init)
	{
		m_VBuffer[0].p		= { 0.0f - CORRECTION, 0.0f - CORRECTION, 0, 1.0f };
		m_VBuffer[0].color	= 0xFFFFFFFF;	m_VBuffer[0].tu		= 0.0f;	m_VBuffer[0].tv		= 0.0f;
		// right top
		m_VBuffer[1].p		= { RGetScreenWidth() + CORRECTION, 0.0f - CORRECTION, 0, 1.0f };
		m_VBuffer[1].color	= 0xFFFFFFFF;	m_VBuffer[1].tu		= 1.0f;	m_VBuffer[1].tv		= 0.0f;
		// right bottom
		m_VBuffer[2].p		= { RGetScreenWidth() + CORRECTION, RGetScreenHeight() + CORRECTION, 0, 1.0f };
		m_VBuffer[2].color	= 0xFFFFFFFF;	m_VBuffer[2].tu		= 1.0f;	m_VBuffer[2].tv		= 1.0f;
		// left bottom
		m_VBuffer[3].p		= { 0.0f - CORRECTION, static_cast<float>(RGetScreenHeight()), 0, 1.0f };
		m_VBuffer[3].color	= 0xFFFFFFFF;	m_VBuffer[3].tu		= 0.0f;	m_VBuffer[3].tv		= 1.0f;

		init = true;
	}

 	RSetWBuffer(FALSE);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

	dev->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TFACTOR);
	dev->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	dev->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  	dev->SetRenderState(D3DRS_TEXTUREFACTOR, 0x05FF7722);

  	dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);

	dev->SetFVF( RTLVertexType );
	dev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, m_VBuffer, sizeof(RTLVertex) );

	RSetWBuffer(TRUE);
}

void ZStencilLight::PostRender()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false );
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true );
	pd3dDevice->SetTexture(0, nullptr);
	pd3dDevice->SetTexture(1, nullptr);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

	pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
} 

void ZStencilLight::Render()
{
	if(m_LightSource.size()<=0) return;

	PreRender();
	for( map<int, LightSource*>::iterator iter = m_LightSource.begin(); iter != m_LightSource.end(); ++iter)
	{			
		LightSource* pLS = iter->second;
		if(!isInViewFrustum(pLS->pos, RGetViewFrustum())) continue;

		D3DLIGHT9 light;
		light.Type			= D3DLIGHT_POINT;
		light.Ambient.r		= 0.1f;
		light.Ambient.g		= 0.1f;
		light.Ambient.b		= 0.1f;
		light.Specular.r	= 1.0f;
		light.Specular.g	= 1.0f;
		light.Specular.b	= 1.0f;
		light.Attenuation0	= 0.05f; 
		light.Attenuation1	= 0.002f; 
		light.Attenuation2	= 0.0f; 

		light.Range			= 300.f;
		light.Position		= pLS->pos;

		float fPower = pLS->bAttenuation ? 
			pLS->power * cos(float(GetGlobalTimeMS()-pLS->attenuationTime)/(pLS->deadTime-pLS->attenuationTime)*.5*PI)
			: pLS->power;
		fPower = min(1.f,max(0.f,fPower));

		light.Diffuse.r		= fPower;
		light.Diffuse.g		= .5*fPower;
		light.Diffuse.b		= .25*fPower;

		ZGetGame()->GetWorld()->GetBsp()->DrawLight(&light);
	}
	
	PostRender();
	return;

	if(m_LightSource.size()<=0) return;

	for( int i = 0; i < SLNITER; ++i )
	{		
		PreRender();
		for( map<int, LightSource*>::iterator iter = m_LightSource.begin(); iter != m_LightSource.end(); ++iter)
		{			
			LightSource* pLS = iter->second;
			if(!isInViewFrustum(pLS->pos, RGetViewFrustum())) continue;
			float radius = SLBASERADIUS*pLS->power;
			for(int j = 0 ; j < i; ++j ) radius = radius*0.95f;
			auto vec = pLS->pos - RCameraPosition;
			if (MagnitudeSq(vec) < radius*radius) bInverse = true;
			RenderStencil( pLS->pos, radius );
		}	
		RenderLight();
	}
	PostRender();
}

int ZStencilLight::AddLightSource(const rvector& p, float power )
{
	LightSource* pNew = new LightSource;
	pNew->bAttenuation = false;
	pNew->pos = p;
	pNew->power = power;
	m_LightSource.insert( map<int,LightSource*>::value_type(m_id,pNew) );
	int rid = m_id;
	m_id = m_id>=MAXLSID?1:m_id+1;
	return rid;
}

int ZStencilLight::AddLightSource(const rvector& p, float power,	DWORD lastTime )
{
	LightSource* pNew = new LightSource;
	pNew->bAttenuation = true;
 	pNew->pos = p;
	pNew->power = power;
	pNew->attenuationTime = GetGlobalTimeMS();
	pNew->deadTime = pNew->attenuationTime + lastTime;
	m_LightSource.insert( map<int,LightSource*>::value_type(m_id,pNew) );
	int rid = m_id;
	m_id = m_id>=MAXLSID?1:m_id+1;
	DMLog("light %d added\n", rid);
	return rid;
}

bool ZStencilLight::SetLightSourcePosition( int id, const rvector& p )
{
	map<int, LightSource*>::iterator iter = m_LightSource.find(id);
	if(iter!=m_LightSource.end()) {
		LightSource* pLS = iter->second;
		if(pLS != NULL)
		{ 
			pLS->pos = p;
			return true;
		}
	}
	return false;
}

bool ZStencilLight::DeleteLightSource( int id )
{
	map<int, LightSource*>::iterator iter = m_LightSource.find(id);
	if(iter!=m_LightSource.end()) {
		LightSource* pLS = iter->second;
		if(pLS != NULL)
		{ 
			SAFE_DELETE(pLS);
			m_LightSource.erase(iter);
			return true;
		}
	}
	return false;
}

bool ZStencilLight::DeleteLightSource( int id, DWORD lastTime )
{
	map<int, LightSource*>::iterator iter = m_LightSource.find(id);
	if(iter!=m_LightSource.end()) {
		LightSource* pLS = iter->second;
		if(pLS != NULL)
		{ 
			pLS->bAttenuation = true;
			pLS->attenuationTime = GetGlobalTimeMS();
			pLS->deadTime = pLS->attenuationTime + lastTime;
			return true;
		}
	}
	return false;
}

void ZStencilLight::Update()
{
	for( map<int, LightSource*>::iterator iter = m_LightSource.begin(); iter != m_LightSource.end(); )
	{
		LightSource* pLS = iter->second;
		if(pLS->bAttenuation)
		{
			if(pLS->deadTime <= GetGlobalTimeMS())
			{
				DMLog("light %d deleted\n",iter->first);
				SAFE_DELETE(pLS);
				iter = m_LightSource.erase(iter);
				continue;
			}
		}
		++iter;
	}
}