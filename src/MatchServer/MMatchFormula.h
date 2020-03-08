#pragma once

#include "MXml.h"
#include <vector>
#include "MMatchItem.h"

#define FILENAME_MATCH_FORMULA		"formula.xml"
#define MAX_LEVEL					99

class MMatchEquipedItem;
class MMatchStageSetting;
class MMatchObject;

class MMatchFormula
{
private:
	static float				m_fNeedExpLMTable[MAX_LEVEL+1];
	static float				m_fGettingExpLMTable[MAX_LEVEL+1];
	static float				m_fGettingBountyLMTable[MAX_LEVEL+1];

	static u32	m_nNeedExp[MAX_LEVEL+1];
	static u32	m_nGettingExp[MAX_LEVEL+1];
	static u32	m_nGettingBounty[MAX_LEVEL+1];

	static bool ReadXml(const char* szXmlFileName);
	static void ParseNeedExpLM(MXmlElement& element);
	static void ParseGettingExpLM(MXmlElement& element);
	static void ParseGettingBountyLM(MXmlElement& element);

	static void PreCalcGettingBounty();
	static void PreCalcNeedExp();
	static void PreCalcGettingExp();
public:
	static bool Create();
	static u32 CalcPanaltyEXP(int nAttackerLevel, int nVictimLevel);
	static u32 GetSuicidePanaltyEXP(int nLevel);
	static u32 GetGettingExp(int nAttackerLevel, int nVictimLevel);
	static u32 GetGettingBounty(int nAttackerLevel, int nVictimLevel);
	static u32 GetNeedExp(int nLevel) { if (nLevel<0) nLevel=0; if (nLevel>MAX_LEVEL) nLevel=MAX_LEVEL; return m_nNeedExp[nLevel]; }
	static int GetLevelFromExp(u32 nExp);

	static int GetLevelPercent(u32 nExp, int nNowLevel);
	static int GetClanBattlePoint(int nWinnerClanPoint, int nLoserClanPoint, int nOneTeamMemberCount);

	static float CalcXPBonusRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType);
	static float CalcBPBounsRatio(MMatchObject* pCharObj, MMatchItemBonusType nBonusType );
};