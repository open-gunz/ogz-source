#include "stdafx.h"

#include "ZGame.h"
#include "ZWeaponMgr.h"
#include "RealSpace2.h"
#include "ZCharacter.h"
#include "ZApplication.h"

#include "ZEffectBillboard.h"
#include "ZEffectSmoke.h"

enum el_plane_type {
	el_plane_yz,//x
	el_plane_xz,//y
	el_plane_xy,//z
};

el_plane_type GetFaceType(rplane& p) {

	el_plane_type mode = el_plane_yz;

	float max = fabs(p.a);//x

	if( fabs(p.b) > max ) { //y
		max = fabs(p.b);
		mode = el_plane_xz;
	}

	if( fabs(p.c) > max ) {//z
		mode = el_plane_xy;
	}

	return mode;
}

////////////////////////////////////////////
// ZWeaponMgr

ZWeaponMgr::ZWeaponMgr() {
	m_fLastTime = 0;
}

ZWeaponMgr::~ZWeaponMgr()
{
	Clear();
}

void ZWeaponMgr::Clear()
{
	if(m_list.size()==0)
		return;

	z_weapon_node node;
	ZWeapon* pWeapon;

	for(node = m_list.begin(); node != m_list.end(); ) {
		pWeapon = (*node);
		delete pWeapon;
		node=m_list.erase(node);
	}
}

void ZWeaponMgr::AddGrenade(const rvector &pos, const rvector &velocity,ZObject* pC)
{
	ZWeaponGrenade* pWeapon = new ZWeaponGrenade;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get("grenade01");
												 
	if(!pMesh) return;

	pWeapon->Create(pMesh,pos,velocity,pC);
	Add(pWeapon);
}

void ZWeaponMgr::AddKit(const rvector &pos, const rvector &velocity,ZCharacter* pC,float Delaytime,
	const char *szMeshName, int nLinkedWorldItemID)
{
	ZWeaponItemkit* pWeapon = new ZWeaponItemkit;

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get(szMeshName);

	if(!pMesh) return;

	pWeapon->Create(pMesh,pos,velocity,pC);
	pWeapon->m_nWorldItemID = nLinkedWorldItemID;
	pWeapon->m_fDelayTime = Delaytime;
	Add(pWeapon);
}

void ZWeaponMgr::AddFlashBang(const  rvector &pos, const rvector &velocity, ZObject* pC)
{
	ZWeaponFlashBang* pWeapon	= new ZWeaponFlashBang;

	RMesh* pMesh	= ZGetWeaponMeshMgr()->Get("flashbang01");

	if( !pMesh )
	{
		return;
	}

	pWeapon->Create( pMesh, pos, velocity, pC );
	Add( pWeapon );
}
//*/
void ZWeaponMgr::AddSmokeGrenade(const rvector &pos, const rvector &velocity,ZObject* pC )
{
	ZWeaponSmokeGrenade* pWeapon	= new ZWeaponSmokeGrenade;

	RMesh* pMesh	= ZGetWeaponMeshMgr()->Get("smoke01");

	if( !pMesh )
	{
		return;
	}

	pWeapon->Create( pMesh, pos, velocity, pC );
	Add( pWeapon );
}

void ZWeaponMgr::AddRocket(const rvector &pos, const rvector &dir,ZObject* pC)
{
	ZWeaponRocket* pWeapon = new ZWeaponRocket;

	// 모델 이름은 스킬 정보에서 얻고..

	RMesh* pMesh = ZGetWeaponMeshMgr()->Get("rocket");

	if(!pMesh) return;

	pWeapon->Create(pMesh,pos,dir,pC);

	Add(pWeapon);
}

///////////////////////////////////////////////////////////////////////////////

void ZWeaponMgr::Render()
{
	Update();

	if(m_list.size() <= 0) return;

	z_weapon_node node,node2;
	ZWeapon* pWeapon;

	for(node = m_list.begin(); node != m_list.end(); node++) {

		pWeapon = (*node);
		pWeapon->Render();
	}
}

void ZWeaponMgr::Update()
{
	float fTime=g_pGame->GetTime();

	if(	m_fLastTime==0 )	// 초기화 안되었음.
		m_fLastTime=fTime;

	float fElapsedTime=fTime-m_fLastTime;
	m_fLastTime=fTime;

	z_weapon_node node;
	ZWeapon* pWeapon;

	for(node = m_list.begin(); node != m_list.end(); ) {

		pWeapon = (*node);

		if( !pWeapon->Update(fElapsedTime) ) {
			delete pWeapon;
			node=m_list.erase(node);
		}
		else ++node;
	}
}

bool ZWeaponMgr::SamePoint(rvector& p1,rvector& p2)
{
	if(p1.x == p2.x && p1.y == p2.y && p1.z == p2.z)
		return true;

	return false;
}

ZWeapon* ZWeaponMgr::GetWorldItem(int nItemID)
{
	z_weapon_node	node;

	ZWeapon* pWeapon = NULL;
	ZMovingWeapon*	pMWeapon = NULL;

	for(node = m_list.begin(); node != m_list.end(); ) {

		pWeapon = (*node);
//		pMWeapon = MDynamicCast( ZMovingWeapon, pWeapon );

		if(pWeapon->GetItemUID()==nItemID ) {
			return pWeapon;
		}

		++node;
	}

	return NULL;
}

ZMovingWeapon* ZWeaponMgr::UpdateWorldItem(int nItemID, const rvector& pos)
{
	z_weapon_node	node;

	ZWeapon* pWeapon;
	ZMovingWeapon*	pMWeapon = NULL;
	ZMovingWeapon*	pMWeaponResult = NULL;

	float _min_weapon = 9999.f;

	for(node = m_list.begin(); node != m_list.end(); ) {

		pWeapon = (*node);

		pMWeapon = MDynamicCast( ZMovingWeapon, pWeapon );

		if(pMWeapon) {
		
//			if(pMWeapon->m_nWorldItemID == nItemID) 
			{
/*
				if( SamePoint(pMWeapon->m_Position,pos) ) {
					return pMWeapon;
				}
				// 가장 가까운 아이템이 자신의것..나중에 서버 패치시.. ID 등을 줘서 식별..
*/				
				float fLen = Magnitude(pMWeapon->m_Position - pos);
				
				if(fLen < 100.f) {
					if(_min_weapon > fLen) {
						_min_weapon = fLen;
						pMWeaponResult = pMWeapon;
					}
				}
			}
		}

		++node;
	}
	
	return pMWeaponResult;
}

void ZWeaponMgr::AddMagic(ZSkill* pSkill, const rvector &pos, const rvector &dir,ZObject* pOwner)
{
	ZWeaponMagic* pWeapon = new ZWeaponMagic;

	RMeshMgr* pMeshMgr = ZGetEffectManager()->GetEffectMeshMgr();

	if(!pMeshMgr) return;

	ZSkillDesc* pDesc = pSkill->GetDesc();

	RMesh* pMesh = pMeshMgr->Get( pDesc->szTrailEffect );

	float fMagicScale = pDesc->fTrailEffectScale;

	if(!pMesh) return;

	pWeapon->Create(pMesh,pSkill,pos,dir,fMagicScale,pOwner);

	Add(pWeapon);
}