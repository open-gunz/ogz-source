#pragma once

#include "MMatchGlobal.h"
#include "MLadderGroup.h"
#include "MLadderStatistics.h"
#include <vector>

class MMatchObject;


class MLadderMgr {
protected:
	int					m_idGenerate;
	u64		m_nLastTick;

	MLadderGroupMap		m_WaitingMaps[MLADDERTYPE_MAX];
	list<MLadderGroup*>	m_GroupList;

	MLadderStatistics	m_Stat;
protected:
	auto GetLastTick()	{ return m_nLastTick; }
	void SetLastTick(u64 nTick)	{ m_nLastTick = nTick; }

	inline MLadderGroupMap* GetWaitGroupContainer(MLADDERTYPE nLadderType);

	void AddGroup(MLADDERTYPE nLadderType, MLadderGroup* pGroup);
	int MakeMatch(MLADDERTYPE nLadderType);
	void LaunchLadder(MLADDERTYPE nLadderType, int nGroupA, int nGroupB);
	void RemoveFromGroupList(MLadderGroup* pGroup);
	void CleaningGarbages();
	u32 GetTickInterval();
public:
	MLadderMgr() {
		m_idGenerate = 0;
	}
	bool Init();
	MLadderGroup* CreateLadderGroup();
	MLadderGroup* FindLadderGroup(int nGroupID);
	bool Challenge(MLadderGroup* pGroup);
	void CancelChallenge(int nGroupID, const char* pszCancelName);

	void Tick(u64 nTick);
	int GenerateID()	{ return ++m_idGenerate; }	
	int GetNeedMemberCount(MLADDERTYPE nLadderType);
	int GetTotalGroupCount();
	u32 GetChecksum(int nFirstIndex, int nGroupCount);

	void DebugTest();
	
	list<MLadderGroup*>::iterator GetGroupListBegin()		{ return m_GroupList.begin(); }
	list<MLadderGroup*>::iterator GetGroupListEnd()			{ return m_GroupList.end(); }
	size_t GetGroupCount()									{ return m_GroupList.size(); }

	MLadderStatistics*	GetStatistics()						{ return &m_Stat; }
};




