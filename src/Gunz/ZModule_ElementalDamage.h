#pragma once

#include "ZModule.h"
#include "ZModuleID.h"
#include "MUID.h"
#include "ZEffectManager.h"

template <void (ZEffectManager::*EffectFunction)(ZObject*), bool Damage = true>
class ZModule_ElementalDamage : public ZModule {
public:
	virtual void OnUpdate(float fElapsed) override final
	{
		assert(MIsDerivedFromClass(ZObject, m_pContainer));

		auto* pObj = MStaticCast(ZObject, m_pContainer);

		if (g_pGame->GetTime() > m_fNextDamageTime) {
			m_fNextDamageTime += DAMAGE_DELAY;

			if (pObj->IsDie()) {
				if (pObj->m_pVMesh->GetVisibility() < 0.5f) {
					Active = false;
					return;
				}
			}
			else if (Damage)
			{
				float fFR = 0;
				float fDamage = 6 * (1.f - fFR) + (float)m_nDamage;

				auto* HPAPModule = static_cast<ZModule_HPAP*>(m_pContainer->GetModule(ZMID_HPAP));
				if (HPAPModule)
					HPAPModule->OnDamage(m_Owner, fDamage, 0);
			}
		}

		if (g_pGame->GetTime() > m_fNextEffectTime) {

			if (!pObj->IsDie())
			{
				int nEffectLevel = GetEffectLevel() + 1;

				m_fNextEffectTime += EFFECT_DELAY * nEffectLevel;

				if (EffectFunction)
					(ZGetEffectManager()->*EffectFunction)(pObj);
			}
		}

		if (m_fNextDamageTime - m_fBeginTime>m_fDuration)
			Active = false;
	}

	void BeginDamage(MUID owner, int nDamage, float fDuration)
	{
		m_fBeginTime = g_pGame->GetTime();
		m_fNextDamageTime = m_fBeginTime + DAMAGE_DELAY;
		m_fNextEffectTime = m_fBeginTime;

		m_Owner = owner;
		m_nDamage = nDamage;
		m_fDuration = fDuration;

		Active = true;
	}

protected:
	float	m_fBeginTime;
	float	m_fNextDamageTime;
	float	m_fNextEffectTime;

	int		m_nDamage;
	float	m_fDuration;
	MUID	m_Owner;

	static constexpr auto DAMAGE_DELAY = 1.0f;
	static constexpr auto EFFECT_DELAY = 0.15f;
};

struct ZModule_FireDamage : public ZModule_ElementalDamage<&ZEffectManager::AddEnchantFire2> {
	DECLARE_ID(ZMID_FIREDAMAGE)
};
struct ZModule_PoisonDamage : public ZModule_ElementalDamage<&ZEffectManager::AddEnchantPoison2> {
	DECLARE_ID(ZMID_POISONDAMAGE)
};
struct ZModule_LightningDamage : public ZModule_ElementalDamage<nullptr> {
	DECLARE_ID(ZMID_LIGHTNINGDAMAGE)
};
struct ZModule_ColdDamage : public ZModule_ElementalDamage<&ZEffectManager::AddEnchantCold2, false> {
	DECLARE_ID(ZMID_COLDDAMAGE)

	void BeginDamage(float fMoveSpeed, float fDuration)
	{
		m_fBeginTime = ZGetGame()->GetTime();
		m_fNextDamageTime = m_fBeginTime + DAMAGE_DELAY;
		m_fNextEffectTime = m_fBeginTime;

		m_fDuration = fDuration;

		auto* pMovableModule = static_cast<ZModule_Movable*>(m_pContainer->GetModule(ZMID_MOVABLE));
		assert(pMovableModule);
		pMovableModule->SetMoveSpeedRatio(fMoveSpeed, fDuration);

		Active = true;
	}
};