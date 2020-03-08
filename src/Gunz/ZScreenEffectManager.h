#pragma once

#include "ZEffectManager.h"
#include "RMeshUtil.h"

enum ZCOMBOLEVEL {
	ZCL_NONE = 0,
	ZCL_GOOD,
	ZCL_NICE,
	ZCL_GREAT,
	ZCL_WONDERFUL
};

class ZScreenEffect : public ZEffect{
protected:
	RealSpace2::RVisualMesh m_VMesh;	
	rvector	m_Offset;

public:
	ZScreenEffect(RMesh* pMesh, rvector offset = rvector(0, 0, 0));
	virtual bool Draw(u64 nTime) override;
	virtual void Update() override;
	virtual bool IsDeleteTime() override;
	bool DrawCustom(u64 nTime, const rvector& offset, float fAngle = 0.0f);
	RealSpace2::RVisualMesh *GetVMesh() { return &m_VMesh; }
	void SetOffset(rvector& offset) { m_Offset = offset; }
};


class ZComboEffect : public ZScreenEffect {
public:
	bool bDelete;
	float fDeleteTime;
	ZComboEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	virtual bool Draw(u64 nTime) override;
	void SetFrame(int nFrame);
	void DeleteAfter(float fTime=0.f);
};

class ZBossGaugeEffect : public ZScreenEffect {
private:
	int			m_nVisualValue;
	bool		m_bShocked;
	float		m_fShockStartTime;
	float		m_fShockPower;
	float		m_fLastTime;
	rvector		m_ShockOffset;
	rvector		m_ShockVelocity;
public:
	ZBossGaugeEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void Shock(float fPower);
	virtual bool Draw(u64 nTime) override;
};

class ZKOEffect : public ZScreenEffect {
public:
	ZKOEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void SetFrame(int nFrame);
	void InitFrame();
	int GetFrame();
};

class ZTDMBlinkEffect : public ZScreenEffect
{
public:
	ZTDMBlinkEffect(RMesh* pMesh,rvector offset=rvector(0,0,0));
	void SetAnimationSpeed(int nKillsDiff);
};



class ZScreenEffectManager : public ZEffectList {
private:
	RMeshMgr*		m_pEffectMeshMgr{};
	RMeshMgr*		m_pQuestEffectMeshMgr{};
	list<ZEffectList::iterator>	m_eraseQueue;

	ZCOMBOLEVEL m_CurrentComboLevel{};

	RMesh*	m_pHit{};
	RMesh*	m_pComboBeginEffect{};
	RMesh*	m_pComboEndEffect{};
	RMesh*	m_pComboNumberEffect[10]{};
	RMesh*	m_pExpPlusEffect{};
	RMesh*	m_pExpMinusEffect{};
	RMesh*	m_pExpNumberEffect[10]{};

	RMesh*	m_pPraiseEffect[ZCI_END]{};

	RMesh*	m_pGoodEffect{};
	RMesh*	m_pNiceEffect{};
	RMesh*	m_pGreatEffect{};
	RMesh*	m_pWonderfullEffect{};

	RMesh*	m_pCoolEffect{};

	RMesh*	m_pAlertEffect[4]{};

	ZScreenEffect*	m_pHPPanel{};
	ZScreenEffect*	m_pScorePanel{};
	ZScreenEffect*	m_pWeaponIcons[MWT_END]{};
	ZScreenEffect*	m_pSpectator{};
	
#define COMBOEFFECTS_COUNT	5
	ZComboEffect	*m_pComboEffects[COMBOEFFECTS_COUNT]{};

	RBaseTexture	*m_pGuageTexture{};

	void DrawGuages();

	float m_fGuageHP{}, m_fGuageAP{}, m_fGuageEXP{};
	float m_fCurGuageHP{}, m_fCurGuageAP{};

	bool m_bGameStart{};

	MMatchWeaponType	m_WeaponType{};
	MMatchItemDesc*		m_SelectItemDesc{};

	int	m_nHpReset{};

	ZScreenEffect*	m_pReload{};
	ZScreenEffect*	m_pEmpty{};

	bool m_bShowReload{};
	bool m_bShowEmpty{};

	ZBossGaugeEffect*	m_pBossHPPanel{};
	ZScreenEffect*		m_pArrow{};
	ZKOEffect*			m_pKONumberEffect[10]{};
	ZKOEffect*			m_pKO{};
	int					m_nKO{};

	ZScreenEffect*		m_pTDScoreBoard{};
	ZTDMBlinkEffect*	m_pTDScoreBlink_R{};
	ZTDMBlinkEffect*	m_pTDScoreBlink_B{};
protected:
	void DrawCombo();
	void PlaySoundScoreFlyby();
	void PlaySoundScoreGet();
	void DrawQuestEffects();
	void DrawDuelEffects();
	void DrawTDMEffects();
	void DrawArrow(rvector& vTargetPos);
	void DrawKO();
public:
	void DrawEffects();

	ZScreenEffectManager();
	~ZScreenEffectManager();

	bool Create();
	void Destroy();
	bool CreateQuestRes();
	void DestroyQuestRes();

	int GetCount() { return (int)size();}

	void Clear();

	void Draw();
	void DrawScoreBoard();
	void DrawSpectator();
	void ResetSpectator();

	int  DrawResetGuages();

	void UpdateEffects();

	void SetGameStart(bool b) {
		m_bGameStart = b;
	}

	void ReSetHpPanel();

	void SetGuage_HP(float fHP); 
	void SetGuage_AP(float fAP);
	void SetGuage_EXP(float fEXP) {	
		m_fGuageEXP=fEXP; 
	}

	void SetGuageExpFromMyInfo();

	void SetWeapon(MMatchWeaponType wtype,MMatchItemDesc* pDesc) {
		m_WeaponType=wtype; 
		m_SelectItemDesc = pDesc;
	}

	void Add(ZEffect *pEffect);

	void AddScreenEffect(RMesh *pMesh,rvector offset=rvector(0,0,0)) { 
		if(pMesh) Add(new ZScreenEffect(pMesh,offset));	
	}

	void AddScreenEffect(char *szEffectName,rvector offset=rvector(0,0,0)) { 
		AddScreenEffect(m_pEffectMeshMgr->Get(szEffectName),offset);	
	}

	ZScreenEffect* CreateScreenEffect(char *szEffectName,rvector offset=rvector(0,0,0)) { 
		return new ZScreenEffect(m_pEffectMeshMgr->Get(szEffectName),offset); 
	}

	void SetCombo(int nCombo);
	void AddExpEffect(int nExp);

	void AddRoundStart(int nRound);

	void AddFinalRoundStart() {
		AddScreenEffect("finalround");
		ZGetGameInterface()->PlayVoiceSound( VOICE_FINAL_ROUND, 1000);
	}

	void AddRoundFinish()		{	AddScreenEffect("finishround"); }
	void AddRock();

	void AddWin()				{	AddScreenEffect("win"); }
	void AddLose()				{	AddScreenEffect("lose"); }
	void AddDraw()				{	AddScreenEffect("draw"); }

	void AddHit()				{	AddScreenEffect("hit"); }

	
	void AddPraise(int nPraise);
	void AddGood();
	void AddNice();
	void AddGreat();
	void AddWonderful();
	void AddCool();

	void ShowReload(bool b) {
		m_bShowReload = b;
	}

	void ShowEmpty(bool b) {
		m_bShowEmpty = b;
	}

	void AddAlert(const rvector& vVictimPos, const rvector& vVictimDir, const rvector& vAttackerPos);
	void AddKO(int nKills=1);
	void SetKO(int nKills);

	void ShockBossGauge(float fPower);

	void UpdateDuelEffects();
};