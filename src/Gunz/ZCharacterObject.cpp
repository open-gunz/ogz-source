#include "stdafx.h"
#include "ZCharacterObject.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "ZActor.h"
#include "RCollisionDetection.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZScreenEffectManager.h"
#include "ZShadow.h"
#include "ZConfiguration.h"
#include "ZModule_Resistance.h"
#include "ZModule_ElementalDamage.h"
#include "RBspObject.h"

MImplementRTTI(ZCharacterObject, ZObject);

struct sCharacterLight
{
	CharacterLight::Type Type;
	float Life;
	v3 LightColor;
	float Range;
};

static const sCharacterLight g_CharLightList[CharacterLight::End] = {
	// Gun
	{
		// Type
		CharacterLight::Gun,
		// Life
		300,
		// LightColor
		v3{5, 1, 1},
		// Range
		100,
	},
	// Shotgun
	{
		// Type
		CharacterLight::Shotgun,
		// Life
		1000,
		// LightColor
		v3{6, 1.3f, 1.3f},
		// Range
		150,
	},
	// Cannon
	{
		// Type
		CharacterLight::Cannon,
		// Life
		1300,
		// LightColor
		v3{7, 1.3f, 1.3f},
		// Range
		200,
	},
};

ZCharacterObject::ZCharacterObject()
{
	m_pSoundMaterial[0] = 0;

#define ADD_MODULE(module) m_pModule_##module = AddModule<ZModule_##module>();
	ADD_MODULE(HPAP);
	ADD_MODULE(Resistance);
	ADD_MODULE(FireDamage);
	ADD_MODULE(ColdDamage);
	ADD_MODULE(PoisonDamage);
	ADD_MODULE(LightningDamage);
#undef ADD_MODULE
}

ZCharacterObject::~ZCharacterObject() = default;

void ZCharacterObject::CreateShadow()
{
	Shadow.emplace();
}

bool ZCharacterObject::GetWeaponTypePos(WeaponDummyType type,rvector* pos,bool bLeft)
{
	if(m_pVMesh && pos) {
		return m_pVMesh->GetWeaponDummyPos(type,pos,bLeft);
	}
	return false;
}

void ZCharacterObject::UpdateEnchant()
{
	ZC_ENCHANT etype = GetEnchantType();
	REnchantType retype = REnchantType_None;

		 if(etype==ZC_ENCHANT_FIRE)			retype = REnchantType_Fire;
	else if(etype==ZC_ENCHANT_COLD)			retype = REnchantType_Cold;
	else if(etype==ZC_ENCHANT_LIGHTNING)	retype = REnchantType_Lightning;
	else if(etype==ZC_ENCHANT_POISON)		retype = REnchantType_Poison;
	else									retype = REnchantType_None;
	
	if(m_pVMesh) {
		m_pVMesh->SetEnChantType(retype);
	}
}

void ZCharacterObject::DrawEnchantSub(ZC_ENCHANT etype,rvector& pos)
{
	if(etype==ZC_ENCHANT_FIRE)
		ZGetEffectManager()->AddTrackFire( pos );
	else if(etype==ZC_ENCHANT_COLD)
		ZGetEffectManager()->AddTrackCold( pos );
	else if(etype==ZC_ENCHANT_POISON)
		ZGetEffectManager()->AddTrackPoison( pos );
}

void ZCharacterObject::EnChantMovingEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon)
{
	int nRand = (GetEffectLevel()+1) * 3;

	if (g_pGame->GetGameTimer()->GetUpdateCount() % nRand == 0) {

		if(cnt==0) { 
			m_pVMesh->GetWeaponPos( pOutPos ); 	
		}

		float asf = (rand()%10)/10.f;
		rvector pos = pOutPos[0] + (pOutPos[1]-pOutPos[0]) * asf;

		DrawEnchantSub( etype , pos );

		if(bDoubleWeapon)
		{
			if(cnt==0) {
				m_pVMesh->GetWeaponPos( &pOutPos[2] ,true);
			}

			float asf = (rand()%10)/10.f;
			rvector pos = pOutPos[2] + (pOutPos[3]-pOutPos[2]) * asf;

			DrawEnchantSub( etype , pos );
		}
	}
}

void ZCharacterObject::EnChantSlashEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon)
{
	if(cnt==0) {
		m_pVMesh->GetWeaponPos( pOutPos );
	}

	float asf = (3 + rand()%3)/10.f;
	rvector pos = pOutPos[0] + (pOutPos[1]-pOutPos[0]) * asf;

	DrawEnchantSub( etype , pos );

	if(bDoubleWeapon)
	{
		if(cnt==0) {
			m_pVMesh->GetWeaponPos( &pOutPos[2] ,true);
		}

		float asf = (3 + rand()%3)/10.f;
		rvector pos = pOutPos[2] + (pOutPos[3]-pOutPos[2]) * asf;

		DrawEnchantSub( etype , pos );
	}
}

void ZCharacterObject::EnChantWeaponEffect(ZC_ENCHANT etype,int nLevel)
{
	float fSwordSize = m_pVMesh->GetWeaponSize();

	fSwordSize -= 35.f;

	ZEffectWeaponEnchant* pEWE = NULL; 

	if( etype != ZC_ENCHANT_NONE )
		pEWE = ZGetEffectManager()->GetWeaponEnchant( etype );

	if(pEWE) {

		float fSIze = fSwordSize / 100.f;

		float fVolSize = 1.f;

			 if(nLevel==0) fVolSize = 0.6f;
		else if(nLevel==1) fVolSize = 0.75f;
		else if(nLevel==2) fVolSize = 1.0f;
		else			   fVolSize = 1.25f;

		rvector vScale = rvector(0.7f*fSIze*fVolSize,0.7f*fSIze*fVolSize,1.1f*fSIze);
		pEWE->SetUid( m_UID );
		pEWE->SetAlignType(1);
		pEWE->SetScale(vScale);
		pEWE->Draw(GetGlobalTimeMS());
	}
}

void ZCharacterObject::DrawEnchant(ZC_STATE_LOWER AniState_Lower,bool bCharged)
{
	ZItem* pItem = GetItems()->GetSelectedWeapon();

	if(!pItem || !pItem->GetDesc()) {
		return;
	}

	MMatchItemDesc* pDesc = pItem->GetDesc();

	if (pDesc->m_nType != MMIT_MELEE) 
		return;

	if(m_pVMesh) {

		static rvector pOutPos[8];

		bool bDoubleWeapon = m_pVMesh->IsDoubleWeapon();

		int cnt = m_pVMesh->GetLastWeaponTrackPos(pOutPos);

		bool bSlash = false;

		if( (AniState_Lower == ZC_STATE_SLASH) ||
			(AniState_Lower == ZC_STATE_JUMP_SLASH1) ||
			(AniState_Lower == ZC_STATE_JUMP_SLASH2) )
			bSlash = true;

		ZC_ENCHANT etype = GetEnchantType();

		MMatchItemDesc* pENDesc = GetEnchantItemDesc();
		if(pENDesc) {

			int nEFLevel = pENDesc->m_nEffectLevel;

			if( (nEFLevel > 2) || ((nEFLevel > 1) && bCharged ) )
			{
				EnChantMovingEffect(pOutPos,cnt,etype,bDoubleWeapon);
			}

			if( bSlash )
			{ 
				EnChantSlashEffect(pOutPos,cnt,etype,bDoubleWeapon);
			}

			if( (nEFLevel > 1) || bCharged )
			{
				EnChantWeaponEffect(etype,nEFLevel);
			}
		}
	}
}

MMatchItemDesc* ZCharacterObject::GetEnchantItemDesc()
{
	for(int i=MMCIP_CUSTOM1;i<=MMCIP_CUSTOM2;i++) {
		ZItem *pItem = m_Items.GetItem((MMatchCharItemParts)i);
		MMatchItemDesc* pDesc = pItem->GetDesc();
		if(pDesc && pDesc->IsEnchantItem() ) return pDesc;
	}

	return NULL;
}

ZC_ENCHANT	ZCharacterObject::GetEnchantType()
{
	MMatchItemDesc* pDesc = GetEnchantItemDesc();
	if(pDesc)
	{
		switch(pDesc->m_nWeaponType)
		{
			case MWT_ENCHANT_FIRE : return ZC_ENCHANT_FIRE;
			case MWT_ENCHANT_COLD : return ZC_ENCHANT_COLD;
			case MWT_ENCHANT_LIGHTNING: return ZC_ENCHANT_LIGHTNING;
			case MWT_ENCHANT_POISON: return ZC_ENCHANT_POISON;
		}
	}

	return ZC_ENCHANT_NONE;
}

void ZCharacterObject::SetGunLight()
{
	constexpr auto CHARACTER_AMBIENT = 0.0;
	D3DLIGHT9 Light{};
	rvector pos;

	Light.Type = D3DLIGHT_POINT;
	Light.Ambient.r = CHARACTER_AMBIENT;
	Light.Ambient.g = CHARACTER_AMBIENT;
	Light.Ambient.b = CHARACTER_AMBIENT;

	Light.Specular.r = 1.f;
	Light.Specular.g = 1.f;
	Light.Specular.b = 1.f;
	Light.Specular.a = 1.f;

	if (ZGetConfiguration()->GetVideo()->bDynamicLight && m_bDynamicLight)
	{
		m_vLightColor.x -= 0.03f;
		m_vLightColor.y -= 0.03f;
		m_vLightColor.z -= 0.03f;
		max(m_vLightColor.x, 0.0f);
		max(m_vLightColor.y, 0.0f);
		max(m_vLightColor.z, 0.0f);
		Light.Diffuse.r = m_vLightColor.x;
		Light.Diffuse.g = m_vLightColor.y;
		Light.Diffuse.b = m_vLightColor.z;
		Light.Range = g_CharLightList[m_iDLightType].Range;

		float lastTime = m_fTime;
		m_fTime = GetGlobalTimeMS();
		float lap = m_fTime - lastTime;
		m_fLightLife -= lap;

		if (m_fLightLife <= 0.0f)
		{
			m_bDynamicLight = false;
			m_fLightLife = 0;
		}
	}
	else
	{
		m_bDynamicLight = false;
		m_vLightColor.x = 0.0f;
		m_vLightColor.y = 0.0f;
		m_vLightColor.z = 0.0f;
	}

	if (IsDoubleGun())
	{
		GetWeaponTypePos(weapon_dummy_muzzle_flash, &pos, m_bLeftShot);

		m_bLeftShot = !m_bLeftShot;
	}
	else
	{
		GetWeaponTypePos(weapon_dummy_muzzle_flash, &pos);
	}

	Light.Position.x = pos.x;
	Light.Position.y = pos.y;
	Light.Position.z = pos.z;

	Light.Attenuation1 = 0.05f;
	Light.Attenuation2 = 0.001f;

	if (IsNPC())
	{
		Light.Ambient.r = 0.4f;
		Light.Ambient.g = 0.4f;
		Light.Ambient.b = 0.4f;
		Light.Range = 2000.0f;

		Light.Attenuation0 = 1.0f;
		Light.Attenuation1 = 0.0f;
		Light.Attenuation2 = 0.0f;
	}

	m_pVMesh->SetLight(0, &Light, false);
}

static RLIGHT* SetMapLight(const v3& char_pos, RVisualMesh* Mesh, int LightIndex, RLIGHT* FirstLight)
{
	D3DLIGHT9 Light{};
	RLIGHT* pSelectedLight{};
	float distance;
	float SelectedLightDistance = FLT_MAX;

	Light.Specular.r = 1.f;
	Light.Specular.g = 1.f;
	Light.Specular.b = 1.f;
	Light.Specular.a = 1.f;

	auto& LightList = ZGetGame()->GetWorld()->GetBsp()->GetSunLightList();
	if (!LightList.empty() && !FirstLight)
	{
		for (auto& Light : LightList)
		{
			auto sunDir = Light.Position - char_pos;
			distance = MagnitudeSq(sunDir);
			Normalize(sunDir);
			RBSPPICKINFO info;
			if (ZGetGame()->GetWorld()->GetBsp()->Pick(char_pos, sunDir, &info, RM_FLAG_ADDITIVE))
			{
				if (distance > MagnitudeSq(char_pos - info.PickPos))
				{
					continue;
				}
			}
			if (distance < SelectedLightDistance)
			{
				SelectedLightDistance = distance;
				pSelectedLight = &Light;
			}
		}
		Light.Type = D3DLIGHT_POINT;
		Light.Attenuation1 = 0.00001f;
		if (pSelectedLight != 0)
		{
			Light.Diffuse.r = pSelectedLight->Color.x * pSelectedLight->fIntensity * 0.15f;
			Light.Diffuse.g = pSelectedLight->Color.y * pSelectedLight->fIntensity * 0.15f;
			Light.Diffuse.b = pSelectedLight->Color.z * pSelectedLight->fIntensity * 0.15f;
		}
	}

	SelectedLightDistance = FLT_MAX;
	if (!pSelectedLight)
	{
		for (auto& Light : ZGetGame()->GetWorld()->GetBsp()->GetObjectLightList())
		{
			float fDist = Magnitude(Light.Position - char_pos);
			if (SelectedLightDistance <= fDist)
				continue;

			if (&Light == FirstLight)
				continue;

			SelectedLightDistance = fDist;
			pSelectedLight = &Light;
		}

		Light.Type = D3DLIGHT_POINT;
		Light.Attenuation1 = 0.0025f;

		if (pSelectedLight)
		{
			Light.Diffuse.r = pSelectedLight->Color.x * pSelectedLight->fIntensity;
			Light.Diffuse.g = pSelectedLight->Color.y * pSelectedLight->fIntensity;
			Light.Diffuse.b = pSelectedLight->Color.z * pSelectedLight->fIntensity;
		}
	}

	if (pSelectedLight)
	{
		Light.Position = pSelectedLight->Position;
		Light.Range = pSelectedLight->fAttnEnd;

		Mesh->SetLight(LightIndex, &Light, false);
	}
	else
		Mesh->SetLight(LightIndex, nullptr, false);

	return pSelectedLight;
}

void ZCharacterObject::Draw_SetLight(const rvector& vPosition)
{
	u32 AmbientColor = 0xCCCCCC;
	RGetDevice()->SetRenderState(D3DRS_AMBIENT, AmbientColor);
	RGetShaderMgr()->setAmbient(AmbientColor);

	if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
	{
		m_pVMesh->SetLight(1, nullptr, false);
		m_pVMesh->SetLight(2, nullptr, false);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		return;
	}

	SetGunLight();

	rvector char_pos = vPosition;
	char_pos.z += 180.f;
	auto* FirstLight = SetMapLight(char_pos, m_pVMesh, 1, nullptr);
	if (FirstLight)
		SetMapLight(char_pos, m_pVMesh, 2, FirstLight);

	RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}

void ZCharacterObject::DrawShadow()
{
	__BP(28, "ZCharacter::Draw::Shadow");

	if(!Shadow.has_value()) return;

	if(!IsDie())
	{
		float fSize = ZShadow::DefaultSize;
		
		ZActor* pActor = MDynamicCast(ZActor,this);

		if(pActor) { 
			fSize = pActor->GetNPCInfo()->fCollRadius * 3.0f;
		}

		if (Shadow.value().SetMatrices(*m_pVMesh, *ZGetGame()->GetWorld()->GetBsp(), fSize))
			Shadow.value().Draw();
	}

	__EP(28);
}


bool ZCharacterObject::IsDoubleGun() { 

	if(m_pVMesh) {
		if(m_pVMesh->m_SelectWeaponMotionType==eq_wd_pistol) {
			return true;
		} else if(m_pVMesh->m_SelectWeaponMotionType==eq_wd_smg) {
			return true;
		}
	}
	return false;
}

int ZCharacterObject::GetWeapondummyPos(rvector* v ) 
{
	if(!v) return 3;

	int size = 3;

	if(!GetWeaponTypePos(weapon_dummy_muzzle_flash,&v[0],false)) {	}
	if(!GetWeaponTypePos(weapon_dummy_cartridge01,&v[1],false)) { v[1]=v[0]; }
	if(!GetWeaponTypePos(weapon_dummy_cartridge02,&v[2],false)) { v[2]=v[0]; }

	if( IsDoubleGun() ) {

		size = 6;

		if(!GetWeaponTypePos(weapon_dummy_muzzle_flash,&v[3],true)) { }
		if(!GetWeaponTypePos(weapon_dummy_cartridge01,&v[4],true))  { v[4]=v[3]; }
		if(!GetWeaponTypePos(weapon_dummy_cartridge02,&v[5],true))  { v[5]=v[4]; }
	}

	return size;
}

bool ZCharacterObject::GetCurrentWeaponDirection(rvector* dir)
{
	if(m_pVMesh && dir) {

		rmatrix* mat = &m_pVMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];

		dir->x = mat->_21;
		dir->y = mat->_22;
		dir->z = mat->_23;

		return true;
	}
	return false;
}


#define MAX_KNOCKBACK_VELOCITY		1700.f

void ZCharacterObject::OnKnockback(const rvector& dir, float fForce)
{
	AddVelocity(dir * fForce);

	rvector vel = GetVelocity();
	if (Magnitude(vel) > MAX_KNOCKBACK_VELOCITY) {
		Normalize(vel);
		vel *= MAX_KNOCKBACK_VELOCITY;
		SetVelocity(vel);
	}

	rvector dir1 = m_Direction;
	rvector dir2 = dir;
	Normalize(dir2);

	float cosAng1 = DotProduct(dir1, dir2);
	float fMaxValue = m_fTremblePower;

	if (cosAng1 < 0.f)	{
		fMaxValue = -fMaxValue;
	}
	Tremble(fMaxValue, 50, 100);
}

void ZCharacterObject::SetLight(CharacterLight::Type Type)
{
	if (Type < 0 || Type > CharacterLight::End)
	{
		assert(!"ZCharacterObject::SetLight -- Type is out of bounds");
		return;
	}

	if (m_bDynamicLight) {
		m_vLightColor = g_CharLightList[Type].LightColor;
		m_fLightLife = g_CharLightList[Type].Life;
	}
	else {
		m_bDynamicLight = true;
		m_vLightColor = g_CharLightList[Type].LightColor;
		m_vLightColor.x = 1.0f;
		m_iDLightType = Type;
		m_fLightLife = g_CharLightList[Type].Life;
	}
}

MImplementRTTI(ZCharacterObjectHistory, ZCharacterObject);

void ZCharacterObjectHistory::EmptyHistory()
{
	m_BasicHistory.clear();
}

bool ZCharacterObjectHistory::GetHistory(rvector *pos, rvector *direction, float fTime, rvector* camerapos)
{
	if (GetVisualMesh() == NULL)
		return false;

	auto SetReturnValues = [&](const rvector& Pos, const rvector& Dir)
	{
		if (isnan(Pos.x) || isnan(Pos.y) || isnan(Pos.z))
			return false;

		if (isnan(Dir.x) || isnan(Dir.y) || isnan(Dir.z))
			return false;

		if (pos)
			*pos = Pos;
		if (direction)
			*direction = Dir;
		if (camerapos)
			*camerapos = Dir;

		return true;
	};

	if (m_BasicHistory.size() > 1)
	{
		auto hi = m_BasicHistory.end();

		ZBasicInfoItem *bi = NULL, *binext = NULL;

		do {
			hi--;
			binext = bi;
			bi = &*hi;
		} while (hi != m_BasicHistory.begin() && bi->fSendTime > fTime);

		if (fTime < bi->fSendTime)
			return false;

		ZBasicInfoItem *pnext;
		ZBasicInfoItem next;

		if (!binext)
		{
			if (fTime >= g_pGame->GetTime())
			{
				return SetReturnValues(m_Position, m_Direction);
			}

			next.fSendTime = g_pGame->GetTime();
			next.info.position = m_Position;
			next.info.direction = m_Direction;
			pnext = &next;
		}
		else
		{
			pnext = binext;
		}

		float t = (fTime - bi->fSendTime) / (pnext->fSendTime - bi->fSendTime);

		return SetReturnValues(Lerp(bi->info.position, pnext->info.position, t),
			Slerp(bi->info.direction, pnext->info.direction, t));
	}
	else
	{
		return SetReturnValues(m_Position, m_Direction);
	}

	return false;
}

void ZCharacterObjectHistory::AddToHistory(const ZBasicInfoItem & Item)
{
	m_BasicHistory.push_back(Item);

	int Surplus = int(m_BasicHistory.size()) - MaxHistorySize;
	if (Surplus > 0)
		m_BasicHistory.erase(m_BasicHistory.begin() + Surplus, m_BasicHistory.end());
}
