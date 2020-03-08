#include "stdafx.h"

#include "RealSpace2.h"
#include "ZEffectLightFragmentList.h"

ZEffectLightFragmentList::ZEffectLightFragmentList(void)
{
	m_fLifeTime = 1.f;
	m_fVanishTime = .5f;
}

ZEffectLightFragmentList::~ZEffectLightFragmentList(void)
{
}

void ZEffectLightFragmentList::Update(float fElapsed)
{
	for(iterator i=begin();i!=end();)
	{
		ZEFFECTBILLBOARDITEM *p=(ZEFFECTBILLBOARDITEM*)*i;

		p->fElapsedTime+=fElapsed;
		if( p->fElapsedTime > m_fLifeTime )
		{
			delete p;
			i=erase(i);
			continue;
		}

		rvector dir=p->velocity;
		Normalize(dir);

		rvector right;
		CrossProduct(&right, dir, RCameraDirection);
		CrossProduct(&p->normal, dir, right);
		CrossProduct(&p->up, p->normal, dir);

		p->velocity += fElapsed*p->accel;
		p->position += fElapsed*p->velocity;
		p->fOpacity = min(1.f, max(0.f, (m_fLifeTime - p->fElapsedTime) / m_fVanishTime));
		i++;
	}
}

void ZEffectLightFragmentList::BeginState()
{
	LPDIRECT3DDEVICE9 pDevice = RGetDevice();

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP	, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP	, D3DTOP_SELECTARG1 );

	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	ZEffectBase::BeginState();
}