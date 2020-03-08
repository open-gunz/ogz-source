#include "stdafx.h"
#include "RealSpace2.h"
#include "RDynamicLight.h"
#include "RShaderMgr.h"

using namespace RealSpace2;

RDynamicLightManager	RDynamicLightManager::msInstance;
sMapLight				RDynamicLightManager::msMapLightList[MAP_LIGHT_NUM];

struct node
{
	node* p;
	node* n;
	float fDist;
	sMapLightObj* pl;
	node() : p(0),n(0),fDist(0),pl(0) {};
};
void compare_and_add( node* first_, node* node_, rvector pos_ );

bool RDynamicLightManager::AddLight( MAP_LIGHT_TYPE light_type_, const rvector& pos_ )
{
	sMapLightObj temp;

	if( light_type_ == GUNFIRE )
	{
		if( mbGunLight )
		{
			mGunLight.fLife					= msMapLightList[GUNFIRE].fLife;
			mGunLight.vLightColor.x += 0.01f;
			mGunLight.vLightColor.y += 0.01f;
			mGunLight.vLightColor.z += 0.01f;
			mGunLight.vLightColor.x = min(mGunLight.vLightColor.x, 1.0f );
			mGunLight.vLightColor.y = min(mGunLight.vLightColor.y, 1.0f );
			mGunLight.vLightColor.z = min(mGunLight.vLightColor.z, 1.0f );
			mGunLight.vPos			= pos_;
		}
		else
		{
			mGunLight.fLife			= msMapLightList[GUNFIRE].fLife;
			mGunLight.vLightColor	= msMapLightList[GUNFIRE].vLightColor;
			mGunLight.fRange		= msMapLightList[GUNFIRE].fRange;
			mGunLight.vPos			= pos_;
		}
		mbGunLight = TRUE;
	}
	else if( light_type_ == EXPLOSION )
	{
		if( mExplosionLightList.size() >= MAX_EXPLOSION_LIGHT )
		{
			return false;
		}
		temp.fLife	= msMapLightList[EXPLOSION].fLife;
		temp.fRange	= msMapLightList[EXPLOSION].fRange;
		temp.vLightColor	= msMapLightList[EXPLOSION].vLightColor;
		temp.vPos = pos_;
		mExplosionLightList.push_back( temp );
	}
	return true;
}

RDynamicLightManager::RDynamicLightManager()
{
	Initialize();
}

RDynamicLightManager::~RDynamicLightManager()
{
	mExplosionLightList.clear();
}

void RDynamicLightManager::Update()
{
	auto lastTime = mTime;
	mTime = float(GetGlobalTimeMS());
	float lap = mTime - lastTime;
	
	if( !mbGunLight && mExplosionLightList.size() <= 0 )
	{
		return;
	}

	if( mbGunLight )
	{
		mGunLight.fLife -= lap;
		if( mGunLight.fLife <= 0 )
		{
			mbGunLight = false;
		}
		mGunLight.vLightColor.x -= 0.01f;
		mGunLight.vLightColor.y -= 0.01f;
		mGunLight.vLightColor.z -= 0.01f;
	}

	for(auto itor = mExplosionLightList.begin(); itor != mExplosionLightList.end(); )
	{
		auto pCurr = &(*itor);
		pCurr->fLife -= lap;
		if( pCurr->fLife <= 0 )
		{
			itor = mExplosionLightList.erase( itor );
		}
		else
		{
			++itor;
		}
	}
}

void RDynamicLightManager::Initialize()
{
	mExplosionLightList.reserve( MAX_EXPLOSION_LIGHT );
}

void RDynamicLightManager::SetPosition(const rvector& pos_ )
{
	mvPosition = pos_;
}

int RDynamicLightManager::SetLight(const rvector& pos_ )
{
	D3DLIGHT9	Light;
	memset( &Light, 0, sizeof(D3DLIGHT9) );
	Light.Type = D3DLIGHT_POINT;
	int available_light_slot;
	int base_slot = 0;
	node result;
	result.fDist = 0;

	if (miNumEnableLight <= 0)
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		return base_slot;
	}

	if( mbGunLight || mExplosionLightList.size() > 0 )
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
		RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00000000 );
		
		RGetShaderMgr()->setAmbient( 0x00000000 );
	}
	else
	{
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	}

	if( mbGunLight )	
	{
		Light.Attenuation1	= 0.00001f;
		Light.Diffuse.r		= mGunLight.vLightColor.x;
		Light.Diffuse.g		= mGunLight.vLightColor.y;
		Light.Diffuse.b		= mGunLight.vLightColor.z;
		Light.Range			= mGunLight.fRange;
		Light.Position.x		= mGunLight.vPos.x;
		Light.Position.y		= mGunLight.vPos.y;
		Light.Position.z		= mGunLight.vPos.z;
		Light.Type			= D3DLIGHT_POINT;

		RGetDevice()->SetLight( 0, &Light );
		RGetDevice()->LightEnable( 0, true );
		available_light_slot = miNumEnableLight - 1;
		base_slot = 1;
	}
	else
	{
		available_light_slot = miNumEnableLight;
		base_slot = 0;
	}

    if( available_light_slot >= 0 )
	{
		float	decisionTest;
		bool	bCull = false;
		rplane* frustum	= &RGetViewFrustum()[0];
		sMapLightObj*	pCurr;

		for( vector<sMapLightObj>::iterator  itor = mExplosionLightList.begin(); 
			itor != mExplosionLightList.end(); ++ itor )
		{
			pCurr = &(*itor);
			
			for( int i = 0 ; i < 6; ++i )
			{
				decisionTest = DotProduct(frustum[i], pCurr->vPos);
				if( decisionTest < 0 )
				{
					bCull = true;
					break;
				}
			}
			if( bCull )
			{
				continue;
			}

			node n;
			n.pl	= pCurr;
			compare_and_add( &result, &n, mvPosition );
		}
	}

	node* pCurr = result.n;
	for( int i = 0 ; i < available_light_slot; ++i )
	{
		if( pCurr == NULL )
		{
			break;
		}
		Light.Attenuation1	= 0.01f;
		Light.Position.x	= pCurr->pl->vPos.x;
		Light.Position.y	= pCurr->pl->vPos.x;
		Light.Position.z	= pCurr->pl->vPos.x;
		
		Light.Diffuse.r		= pCurr->pl->vLightColor.x;
		Light.Diffuse.g		= pCurr->pl->vLightColor.y;
		Light.Diffuse.b		= pCurr->pl->vLightColor.z;
		Light.Range			= pCurr->pl->fRange;
		Light.Type			= D3DLIGHT_POINT;
		
		pCurr->pl->bUsing = TRUE;

		RGetDevice()->LightEnable( base_slot, TRUE );
		RGetDevice()->SetLight( base_slot++, &Light );		
	}

	return base_slot;
}

bool RDynamicLightManager::SetLight(const rvector& pos_, int lightIndex_, float maxDistance_  )
{
	return true;
}

void RDynamicLightManager::ReleaseLight()
{
	for( int i = 0 ; i < miNumEnableLight; ++i )
	{
		RGetDevice()->LightEnable( i, FALSE );
	}
}

RDynamicLightManager* RGetDynamicLightManager()
{
	return RDynamicLightManager::GetInstance();
}

sMapLight*	RGetMapLightList()
{
	return RDynamicLightManager::GetLightMapList();
}

#define DISTANCE_CLOSE	50

void compare_and_add( node* first_, node* node_, rvector pos_)
{
	node*	pCurr	= first_;
	node*	pTemp;
	float	Dist;
	while( pCurr != NULL )
	{
		Dist = fabs( pCurr->fDist - node_->fDist );

		if( pCurr->fDist > node_->fDist )
		{
			pTemp = pCurr;
			pCurr = node_;
			pCurr->n = pTemp;

			return;
		}
		else if( Dist < DISTANCE_CLOSE )
		{
			if( !pCurr->pl->bUsing && node_->pl->bUsing )
			{
				pTemp = pCurr;
				pCurr = node_;
				pCurr->n = pTemp;

				return;
			}
			if( pCurr->pl->fLife < node_->pl->fLife )
			{
				pTemp = pCurr;
				pCurr = node_;
				pCurr->n = pTemp;

				return;
			}
		}
		
		pCurr = pCurr->n;
	}

	pCurr = node_;
}

sMapLightObj::sMapLightObj()
{
	bUsing = false;
};
