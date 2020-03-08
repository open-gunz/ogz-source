#pragma once

#include <memory>
#include <list>

#include "ZObject.h"
#include "stuff.h"
#include "BasicInfoHistory.h"
#include "optional.h"
#include "ZShadow.h"

namespace RealSpace2
{
struct RLIGHT;
}

namespace CharacterLight
{
enum Type
{
	Gun,
	Shotgun,
	Cannon,
	End,
};
}

class ZCharacterObject : public ZObject
{
	MDeclareRTTI;
public:
	ZCharacterObject();
	~ZCharacterObject();

	void CreateShadow();

	bool GetWeaponTypePos(WeaponDummyType type,rvector* pos,bool bLeft=false);

	int GetWeapondummyPos(rvector* pVec);

	bool GetCurrentWeaponDirection(rvector* dir);

	void UpdateEnchant();

	void DrawEnchantSub(ZC_ENCHANT etype,rvector& pos);

	void DrawEnchant(ZC_STATE_LOWER AniState_Lower,bool bCharged);
	void EnChantWeaponEffect(ZC_ENCHANT etype,int nLevel);
	void EnChantSlashEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon);
	void EnChantMovingEffect(rvector* pOutPos,int cnt,ZC_ENCHANT etype,bool bDoubleWeapon);

	MMatchItemDesc* GetEnchantItemDesc();
	ZC_ENCHANT	GetEnchantType();

	void DrawShadow();
	void Draw_SetLight(const rvector& vPosition);

	bool IsDoubleGun();

	void SetHero(bool bHero = true) { m_bHero = bHero; }
	bool IsHero() const { return m_bHero; }

	virtual void OnKnockback(const rvector& dir, float fForce);
	void SetTremblePower(float fPower) { m_fTremblePower = fPower; }

	void SetLight(CharacterLight::Type Type);

	const char* GetSoundMaterial() const { return m_pSoundMaterial; }

protected:
	ZModule_HPAP			*m_pModule_HPAP{};
	ZModule_Resistance		*m_pModule_Resistance{};
	ZModule_FireDamage		*m_pModule_FireDamage{};
	ZModule_ColdDamage		*m_pModule_ColdDamage{};
	ZModule_PoisonDamage	*m_pModule_PoisonDamage{};
	ZModule_LightningDamage	*m_pModule_LightningDamage{};

	bool m_bHero{};

	char	m_pSoundMaterial[16];
	bool	m_bLeftShot{};
	float	m_fTime{};
	int		m_iDLightType{};
	float	m_fLightLife{};
	rvector	m_vLightColor{};

	optional<ZShadow> Shadow;
	bool m_bDynamicLight{};

private:
	void SetGunLight();

	float m_fTremblePower = 30;
};

struct ZBasicInfoItem
{
	ZBasicInfo info;
	float fReceivedTime;
	float fSendTime;
};

using ZBasicInfoHistory = std::vector<ZBasicInfoItem>;

class ZCharacterObjectHistory : public ZCharacterObject
{
	MDeclareRTTI;
public:
	virtual bool GetHistory(rvector *pos, rvector *direction, float fTime, rvector* camerapos = nullptr) override;
	void AddToHistory(const ZBasicInfoItem& Item);
	void EmptyHistory();

private:
	ZBasicInfoHistory m_BasicHistory;
	static constexpr int MaxHistorySize = 100;
};
