#ifndef _ZQUEST_GAMEINFO_H
#define _ZQUEST_GAMEINFO_H

#include "MBaseQuest.h"
#include <vector>
using namespace std;

// 퀘스트의 게임 정보
class ZQuestGameInfo
{
private:
	bool							m_bInitialized;
	vector<MQUEST_NPC>				m_NPCInfoVector;
	vector<MQuestLevelSectorNode>	m_MapSectorVector;
	int								m_nQL;						// 현재 시나리오의 퀘스트 레벨
	float							m_fNPC_TC;
	int								m_nNPCCount;				// 한 섹터당 등장할 총 NPC수
	int								m_nNPCKilled;				// 한 섹터당 죽은 NPC 수
	int								m_nCurrSectorIndex;			// 현재 게임중인 섹터 인덱스
	int								m_nNumOfObtainQuestItem;	// 퀘스트 팀이 획득한 아이템 갯수
	vector<MUID>					m_Bosses;					// 보스 UID
	rvector							m_vPortalPos;				// 포탈의 위치
public:
	ZQuestGameInfo();
	~ZQuestGameInfo();
	void Init(MTD_QuestGameInfo* pMTDQuestGameInfo);
	void Final();
	void OnMovetoNewSector(int nSectorIndex);				// 새로운 섹터로 이동

	// interface func
	int GetNPCInfoCount();
	int GetMapSectorCount();
	MQUEST_NPC GetNPCInfo(int index);
	int GetMapSectorID(int index);
	int GetMapSectorLink(int index);
	bool IsInited();
	inline float GetNPC_TC();
	inline bool IsCurrSectorLastSector();			// 현재 섹터가 마지막 섹터인지 여부
	int GetCurrSectorIndex()				{ return m_nCurrSectorIndex; }
	int GetNPCCount( void)					{ return m_nNPCCount; }
	int GetNPCKilled( void)					{ return m_nNPCKilled; }
	void ClearNPCKilled( void)				{ m_nNPCKilled = 0; }
	void IncreaseNPCKilled( void)			{ m_nNPCKilled++; }
	void SetQuestLevel( int nQL)			{ m_nQL = nQL; }
	int GetQuestLevel( void)				{ return m_nQL; }
	int GetNumOfObtainQuestItem( void)		{ return m_nNumOfObtainQuestItem; }
	void IncreaseObtainQuestItem( void)		{ m_nNumOfObtainQuestItem++; }
	vector<MUID>& GetBosses(void)			{ return m_Bosses; }
	MUID GetBoss();
	void SetPortalPos(rvector& pos)			{ m_vPortalPos = pos; }
	rvector GetPortalPos()					{ return m_vPortalPos; }
};



/////////////////////////////////////////////////////////////////////////////////////
inline int ZQuestGameInfo::GetNPCInfoCount() 
{ 
	return (int)m_NPCInfoVector.size(); 
}
inline int ZQuestGameInfo::GetMapSectorCount() 
{ 
	return (int)m_MapSectorVector.size(); 
}
inline MQUEST_NPC ZQuestGameInfo::GetNPCInfo(int index) 
{ 
	return m_NPCInfoVector[index]; 
}
inline int ZQuestGameInfo::GetMapSectorID(int index) 
{ 
	return m_MapSectorVector[index].nSectorID; 
}
inline int ZQuestGameInfo::GetMapSectorLink(int index)
{
	return m_MapSectorVector[index].nNextLinkIndex;
}

inline bool ZQuestGameInfo::IsInited()
{
	return m_bInitialized; 
}

inline float ZQuestGameInfo::GetNPC_TC()
{
	return m_fNPC_TC;
}

bool ZQuestGameInfo::IsCurrSectorLastSector()
{
	if (GetMapSectorCount() <= (m_nCurrSectorIndex+1)) return true;
	return false;
}

#endif