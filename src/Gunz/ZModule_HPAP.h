#pragma once

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_HPAP : public ZModule {
private:
	float		m_nMaxHP = 100.0f;
	float		m_nMaxAP = 100.0f;
	float		fHP = 1000.0f;
	float		fAP = 1000.0f;
	bool		m_bRealDamage = false;
	MUID		m_LastAttacker{};

	bool	CheckQuestCheet();
public:
	DECLARE_ID(ZMID_HPAP)

	virtual void InitStatus() override;

	auto GetMaxHP() const { return static_cast<int>(m_nMaxHP); }
	void SetMaxHP(int nMaxHP) { m_nMaxHP = static_cast<float>(nMaxHP); }
	auto GetMaxAP() const { return static_cast<int>(m_nMaxAP); }
	void SetMaxAP(int nMaxAP) { m_nMaxAP = static_cast<float>(nMaxAP); }

	auto GetHP() const { return static_cast<int>(fHP); }
	void SetHP(int nHP);
	auto GetAP() const { return static_cast<int>(fAP); }
	void SetAP(int nAP);

	bool IsFullHP() const { return GetHP() == m_nMaxHP; }
	bool IsFullAP() const { return GetAP() == m_nMaxAP; }

	void SetRealDamage(bool bReal) { m_bRealDamage = bReal; }

	void SetLastAttacker(const MUID& uid) { m_LastAttacker = uid; }
	auto& GetLastAttacker() const { return m_LastAttacker; }

	void OnDamage(MUID uidAttacker,int damage, float fRatio);
};