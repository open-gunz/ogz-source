#include "stdafx.h"
#include "ZMyInfo.h"

ZMyInfo::ZMyInfo() : m_bNewbie(false), m_nPGradeID(MMPG_FREE), m_nUGradeID(MMUG_FREE)
{
	Clear();
	m_bCreated = false;
}

ZMyInfo::~ZMyInfo()
{
	Destroy();
}

ZMyInfo* ZMyInfo::GetInstance()
{
	static ZMyInfo m_stMyInfo;
	return &m_stMyInfo;
}

bool ZMyInfo::InitCharInfo(const char* szCharName,
	const char* szClanName,
	MMatchClanGrade nClanGrade,
	MMatchSex nSex,
	int nHair,
	int nFace)
{
	if (m_bCreated == true) 
	{
		Destroy();
	}
	m_ItemList.Create();

	strcpy_safe(m_szCharName, szCharName);
	strcpy_safe(m_szClanName, szClanName);
	m_nSex = nSex;
	m_nHair = nHair;
	m_nFace = nFace;
	m_nClanGrade = nClanGrade;

	m_bCreated = true;
	return true;
}
void ZMyInfo::Destroy()
{
	if (m_bCreated == false) return;

	m_ItemList.Destroy();


	m_bCreated = false;
}
void ZMyInfo::Clear()
{
	m_szAccountID[0] = 0;
	m_szCharName[0] = 0;
	m_szClanName[0] = 0;

	m_nSex = MMS_MALE;
	m_nHair = 0;
	m_nRace = 0;
	m_nFace = 0;

	m_nXP = 0;
	m_nBP = 0;
	m_nLevelPercent = 0;
	m_nLevel = 0;

	m_ItemList.Clear();
}

void ZMyInfo::SetClanInfo(const char* szClanName, const MMatchClanGrade nClanGrade)
{
	strcpy_safe(m_szClanName, szClanName);
	m_nClanGrade = nClanGrade;
}

int ZMyInfo::GetHP()
{
	return (m_ItemList.GetEquipedHPModifier() + DEFAULT_CHAR_HP);
}

int ZMyInfo::GetAP()
{
	return (m_ItemList.GetEquipedAPModifier() + DEFAULT_CHAR_AP);
}

#define NEWBIE_MAX_LEVEL			5

void ZMyInfo::SetLevel( int nLevel )
{ 
	m_nLevel = nLevel;
	if (IsNewbie())
	{
		if (m_nLevel > NEWBIE_MAX_LEVEL)
		{
			SetNewbie(false);
		}
	}
}

bool ZMyInfo::InitAccountInfo(const char* szAccountID, MMatchUserGradeID nUGradeID, MMatchPremiumGradeID nPGradeID)
{
	strcpy_safe(m_szAccountID, szAccountID);
	m_nUGradeID = nUGradeID;
	m_nPGradeID = nPGradeID;

	return true;
}