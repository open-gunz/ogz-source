#pragma once

class ZMatch;

class ZRule
{
protected:
	ZMatch*		m_pMatch;
	virtual void OnUpdate(float fDelta) {}
public:
	ZRule(ZMatch* pMatch);
	virtual ~ZRule();
	void Update(float fDelta);
	virtual bool OnCommand(MCommand* pCommand);
	virtual void OnResponseRuleInfo(MTD_RuleInfo* pInfo);
	static ZRule* CreateRule(ZMatch* pMatch, MMATCH_GAMETYPE nGameType);
};