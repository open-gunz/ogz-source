#pragma once

#include "ZPrerequisites.h"
#include "ZMyItemList.h"

// HShield stuff
// TODO: Remove
#define SIZEOF_REQMSG		160
#define SIZEOF_ACKMSG		56
#define SIZEOF_GUIDREQMSG	20
#define SIZEOF_GUIDACKMSG	20

struct ZMySystemInfo
{
	bool			bInGameNoChat;
	char			szSerialKey[128];			// XTrap RandomValue
	unsigned char	pbyAckMsg[SIZEOF_ACKMSG];			// HShield CRC
	unsigned char	pbyGuidAckMsg[SIZEOF_GUIDACKMSG];	// HShield GUID

	ZMySystemInfo()
	{
		bInGameNoChat = false;
		memset(szSerialKey, 0, sizeof(szSerialKey));
		memset(pbyAckMsg, 0, sizeof(pbyAckMsg));
		memset(pbyGuidAckMsg, 0, sizeof(pbyGuidAckMsg));
	}
};

struct ZMyGameInfo
{
	bool bForcedChangeTeam{};

	void InitRound()
	{
		bForcedChangeTeam = false;
	}
};

class ZMyInfo
{
private:
	bool			m_bCreated{};
	ZMySystemInfo	m_MySystemInfo;
	ZMyGameInfo		m_MyGameInfo;
protected:
	char					m_szAccountID[256];
	MMatchUserGradeID		m_nUGradeID = MMUG_FREE;
	MMatchPremiumGradeID	m_nPGradeID = MMPG_FREE;

	char			m_szCharName[MATCHOBJECT_NAME_LENGTH];
	char			m_szClanName[CLAN_NAME_LENGTH];
	MMatchSex		m_nSex = MMS_MALE;
	int				m_nHair{};
	int				m_nFace{};
	int				m_nRace{};
	u64				m_nXP{};
	int				m_nBP{};
	int				m_nLevel{};
	int				m_nLevelPercent{};
	MMatchClanGrade	m_nClanGrade = MCG_NONE;
	bool			m_bNewbie{};

	ZMyItemList		m_ItemList;


#ifdef _QUEST_ITEM
	ZMyQuestItemMap	m_QuestItemMap;
	ZMyQuestItemMap m_ObtainQuestItemMap;

public:
	ZMyQuestItemMap& GetQuestItemMap()			{ return m_QuestItemMap; }
	ZMyQuestItemMap& GetObtainQuestItemMap()	{ return m_ObtainQuestItemMap; }
#endif

	void Clear();
public:
	ZMyInfo();
	virtual ~ZMyInfo();
	bool InitCharInfo(const char* szCharName, const char* szClanName, const MMatchClanGrade nClanGrade, const MMatchSex nSex, const int nHair, const int nFace);
	bool InitAccountInfo(const char* szAccountID, MMatchUserGradeID nUGradeID, MMatchPremiumGradeID nPGradeID);
	void Destroy();

	static ZMyInfo*		GetInstance();
	ZMyItemList*		GetItemList() { return &m_ItemList; }
	ZMySystemInfo*		GetSystemInfo()	{ return &m_MySystemInfo; }
	ZMyGameInfo*		GetGameInfo() { return &m_MyGameInfo; }
	MMatchSex			GetSex() const { return m_nSex; }
	int					GetHair() const { return m_nHair; }
	int					GetFace() const { return m_nFace; }
	int					GetRace() const { return m_nRace; }
	int					GetLevel() const { return m_nLevel;}
	int					GetLevelPercent() const { return m_nLevelPercent; }
	const char*			GetCharName() const { return m_szCharName; }
	const char*			GetClanName() const { return m_szClanName; }
	const char*			GetAccountID() const { return m_szAccountID; }
	auto				GetXP() const { return m_nXP; }
	int					GetBP() const { return m_nBP; }
	int					GetHP();
	int					GetAP();
	bool				IsNewbie()	{ return m_bNewbie; }
	MMatchUserGradeID	GetUGradeID() const	{ return m_nUGradeID; }
	MMatchPremiumGradeID	GetPGradeID() const { return m_nPGradeID; }
	MMatchClanGrade		GetClanGrade() const { return m_nClanGrade; }

	bool IsAdminGrade() const
	{
		switch (GetUGradeID())
		{
		case MMUG_EVENTMASTER:
		case MMUG_DEVELOPER:
		case MMUG_ADMIN:
			return true;
		default:
			return false;
		}
	}

	bool IsPremiumIPUser() const { return (m_nPGradeID == MMPG_PREMIUM_IP); }
	bool IsClanJoined() const { return ((m_szClanName[0] == 0) ? false : true); }

	void SetXP(u32 nXP)			{ m_nXP = nXP; }
	void SetBP(int nBP)							{ m_nBP = nBP; }
	void SetLevel( int nLevel );
	void SetLevelPercent(int nPercent)			{ m_nLevelPercent = nPercent; }
	void SetClanInfo(const char* szClanName, const MMatchClanGrade nClanGrade);
	void SetNewbie(bool bNewbie)				{ m_bNewbie = bNewbie; }
};

inline ZMyInfo* ZGetMyInfo() { return ZMyInfo::GetInstance(); }
