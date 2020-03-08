#include "stdafx.h"

#include "ZGame.h"
#include "ZApplication.h"
#include "ZEffectStaticMesh.h"
#include "Physics.h"
#include "ZCharacter.h"
#include "ZObject.h"
#include "RMaterialList.h"
#include "MDebug.h"

ZEffectMesh::ZEffectMesh(RMesh* pMesh, const rvector& Pos, const rvector& Velocity)
{
	m_VMesh.Create(pMesh);

	m_Pos = Pos;
	m_Velocity = Velocity;
	m_fRotateAngle = 0;
	m_nStartTime = GetGlobalTimeMS();

	while(1) {

		m_RotationAxis = rvector((rand()%100), (rand()%100), (rand()%100));

		if(m_RotationAxis.x!=0 && m_RotationAxis.y!=0 && m_RotationAxis.z!=0){
			Normalize(m_RotationAxis);
			break;
		}
	}

	m_nDrawMode = ZEDM_NONE;

	m_Up = rvector(0.f,0.f,1.f);
}

ZEffectStaticMesh::ZEffectStaticMesh(RMesh* pMesh, const rvector& Pos, const rvector& Velocity, MUID uid )
: ZEffectMesh(pMesh,Pos,Velocity)
{
	m_uid = uid;
}

#define EC_ROTATION	1.2f
#define EC_LIFETIME	1000
#define EC_LIMIT_DISTANCE	500.f

bool ZEffectStaticMesh::Draw(u64 nTime)
{
	if(m_VMesh.m_pMesh==NULL) 
		return false;

	auto dwDiff = nTime - m_nStartTime;

	float fSec = (float)dwDiff/1000.0f;
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;
	rvector Pos = m_Pos + Distance;
	float fOpacity = (EC_LIFETIME-dwDiff)/(float)EC_LIFETIME;

	rvector Dir(1,0,0);
	rvector Up = m_Up;
	rmatrix World;
	u32 Opacity = 0xFF*fOpacity;
	MakeWorldMatrix(&World, Pos, Dir, Up);
	rmatrix Rotation = RotationMatrix(m_RotationAxis, m_fRotateAngle);
	m_fRotateAngle+=EC_ROTATION;
 	World = Rotation * World;

	m_VMesh.SetWorldMatrix(World);

	if(m_bRender) {
		m_VMesh.Render();
		m_bisRendered = m_VMesh.m_bIsRender;
	} 
	else {
		m_bisRendered = false;
	}

	static const char* base_snd_name = "fx_slugdrop_";
	static char buffer[64];
	
	if( dwDiff > EC_LIFETIME )
	{
		ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);
		ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pObj);
		if(!pCObj) return false;

		auto* SoundMaterial = pCObj->GetSoundMaterial();

		auto DefaultSound = "fx_slugdrop_mt_con";

		if(SoundMaterial == nullptr || SoundMaterial[0] == 0)
		{
			ZGetSoundEngine()->PlaySound(DefaultSound, Pos);
		} 
		else
		{
			strcpy_safe(buffer, base_snd_name);
			strcat_safe(buffer, SoundMaterial);

			ZGetSoundEngine()->PlaySoundElseDefault(buffer, DefaultSound, Pos);
		}
		return false;
		
	}
	return true;
}
