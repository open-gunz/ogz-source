#pragma once

#include "MMatchGlobal.h"
#include <list>
#include <map>
#include "MUID.h"

class MCommand;
class MCommandCommunicator;
class MMatchObject;

struct MMatchObjCacheCostume
{
	MMatchSex			nSex;
	unsigned char		nHair;
	unsigned char		nFace;
	u32					nEquipedItemID[MMCIP_END];
};

class MMatchObjCache {
protected:
	// For backwards compatibility, since this type used to have a virtual destructor.
	u32 dummy_vtable_standin;
	MUID					m_uidObject;
	char					m_szName[32];
	char					m_szClanName[CLAN_NAME_LENGTH];
	char					m_nLevel;
	MMatchUserGradeID		m_nUGrade;
	MMatchPremiumGradeID	m_nPGrade;
	unsigned char			m_nPlayerFlags;
	unsigned int			m_nCLID;
	unsigned int			m_nEmblemChecksum;
	MMatchObjCacheCostume	m_Costume;
	
public:
	MMatchObjCache()				{ 
		m_szName[0] = 0;
		m_nLevel = 0;
		m_nUGrade = MMUG_FREE;
		m_nPGrade = MMPG_FREE;
		memset(&m_Costume, 0, sizeof(MMatchObjCacheCostume));
		ResetFlag();
	}
	~MMatchObjCache()		{}

	MUID GetUID()					{ return m_uidObject; }

	const char* GetName() const { return m_szName; }
	const char* GetClanName() const { return m_szClanName; }
	int GetLevel()					{ return m_nLevel; }
	MMatchUserGradeID		GetUGrade()	{ return m_nUGrade; }
	MMatchPremiumGradeID	GetPGrade()	{ return m_nPGrade; }
	
	void SetInfo(const MUID& uid, const char* szName, const char* szClanName, int nLevel, 
				 MMatchUserGradeID nUGrade, MMatchPremiumGradeID nPGrade)
	{
		m_uidObject = uid;
		strcpy_safe(m_szName, szName);
		strcpy_safe(m_szClanName, szClanName);
		m_nLevel = (char)nLevel;
		m_nUGrade = nUGrade;
		m_nPGrade = nPGrade;
		m_nPlayerFlags = 0;
	}

	unsigned char GetFlags()				{ return m_nPlayerFlags; }
	void SetFlags(unsigned char nFlags)		{ m_nPlayerFlags = nFlags; }
	bool CheckFlag(unsigned char nFlagIdx)	{ return (m_nPlayerFlags & nFlagIdx) ? true:false; }
	void ResetFlag()						{ m_nPlayerFlags = 0; }
	void SetFlag(unsigned char nFlagIdx, bool bSet)	
	{ 
		if (bSet) m_nPlayerFlags |= nFlagIdx; 
		else m_nPlayerFlags &= (0xff ^ nFlagIdx);
	}
	unsigned int GetCLID()					{ return m_nCLID; }
	void SetCLID(unsigned int nCLID)		{ m_nCLID = nCLID; }
	unsigned int GetEmblemChecksum()		{ return m_nEmblemChecksum; }
	void SetEmblemChecksum(unsigned int nChecksum)	{ m_nEmblemChecksum = nChecksum; }

	void AssignCostume(MMatchObjCacheCostume* pCostume) { memcpy(&m_Costume, pCostume, sizeof(MMatchObjCacheCostume)); }
	MMatchObjCacheCostume* GetCostume() { return &m_Costume; }
};

using MMatchObjCacheList = std::list<MMatchObjCache*>;
class MMatchObjCacheMap : public std::map<MUID, MMatchObjCache*>{
public:
	void Insert(const MUID& uid, MMatchObjCache* pCache)	{	
		insert(value_type(uid, pCache));	
	}
};

enum MATCHCACHEMODE {
	MATCHCACHEMODE_ADD = 0,
	MATCHCACHEMODE_REMOVE,
	MATCHCACHEMODE_UPDATE,
	MATCHCACHEMODE_REPLACE
};