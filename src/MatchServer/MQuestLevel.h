#ifndef _MQUEST_LEVEL_H
#define _MQUEST_LEVEL_H

#include "MBaseQuest.h"
#include "MQuestScenario.h"
#include "MQuestItem.h"
#include <vector>
#include <map>
#include <set>
using namespace std;

struct MTD_QuestGameInfo;
struct MQuestNPCSetInfo;




/// 퀘스트 월드레벨에서 나올 NPC들
class MQuestNPCQueue
{
private:
	vector<MQUEST_NPC>			m_Queue;			///< NPC가 vector형태로 들어있다.
	int							m_nCursor;
public:
	MQuestNPCQueue();										///< 생성자
	~MQuestNPCQueue();										///< 소멸자

	/// QLD와 NPCSet를 바탕으로 NPC큐를 만든다.
	/// @param nQLD				QLD
	/// @param pNPCSetInfo		NPC Set 정보
	void Make(int nQLD, MQuestNPCSetInfo* pNPCSetInfo, MQUEST_NPC nKeyNPC=NPC_NONE);		
	void Clear();											///< 큐 초기화
	bool Pop(MQUEST_NPC& outNPC);							///< 큐에서 NPC를 하나 꺼낸다.
	bool GetFirst(MQUEST_NPC& outNPC);						///< 제일 처음 NPC값을 읽는다.
	bool IsEmpty();											///< 큐가 비었는지 체크
	int GetCount();											///< 큐에 들어있는 NPC수 반환. Pop으로 이미 꺼낸 NPC도 포함한다.
};




/// 변하지 않는 퀘스트 월드레벨 정보
struct MQuestLevelStaticInfo
{
	MQuestScenarioInfo*				pScenario;			///< 해당 레벨의 시나리오
	int								nDice;				///< 굴린 주사위수
	set<MQUEST_NPC>					NPCs;				///< 나올 NPC 종류
	vector<MQuestLevelSectorNode>	SectorList;			///< Map Sector 리스트
	int								nQL;				///< 퀘스트 레벨
	float							fNPC_TC;			///< NPC 난이도 조절 계수(NPC Toughness Coefficient)
	int								nQLD;				///< 퀘스트 레벨 난이도
	int								nLMT;				///< 한번에 스폰되는 맙의 숫자 제한

	MQuestLevelStaticInfo()
	{
		pScenario=NULL;
		nDice = 0;
		nQL=0;
		fNPC_TC = 1.0f;
		nQLD = 0;
		nLMT = 1;
	}
};

/// 퀘스트에서 나온 아이템
struct MQuestLevelItem
{
	u32	nItemID;			///< 퀘스트 아이템 ID
	int					nRentPeriodHour;	///< 일반 아이템일 경우 사용 기간
	bool				bObtained;			///< 플레이어가 먹었는지 여부
	int					nMonsetBibleIndex;	// 몬스터 도감에 사용될 몬스터 타입의 고유 인덱스.

	MQuestLevelItem() : nItemID(0), bObtained(false), nRentPeriodHour(0) {}
	bool IsQuestItem() { return IsQuestItemID(nItemID); }
};

/// 퀘스트에서 얻은 아이템들
class MQuestLevelItemMap : public multimap<u32, MQuestLevelItem*>
{
public:
	MQuestLevelItemMap() {}
	~MQuestLevelItemMap() { Clear(); }
	void Clear()
	{
		for (iterator itor = begin(); itor != end(); ++itor)
		{
			delete (*itor).second;
		}
		clear();
	}
};


/// 퀘스트 레벨 진행하면서 변하는 정보
struct MQuestLevelDynamicInfo
{
	int						nCurrSectorIndex;								///< 현재 진행중인 섹터 인덱스
	MQuestMapSectorInfo*	pCurrSector;									///< 현재 진행중인 섹터
	int						nQLD;											///< 퀘스트 레벨 난이도 상수(QLD)
	bool					bCurrBossSector;								///< 현재 섹터가 보스 섹터인지 여부
	MQuestLevelItemMap		ItemMap;										///< 퀘스트에서 나온 아이템들

	MQuestLevelDynamicInfo()
	{
		nCurrSectorIndex = 0;
		pCurrSector = NULL;
		nQLD = 0;
		bCurrBossSector = false;
	}
};


/// 퀘스트 월드 레벨 - 퀘스트의 모든 정보가 여기 다 있다.
class MQuestLevel
{
private:
	/// NPC 스폰 시간을 조정하기 위한 구조체
	struct MQuestLevelSpawnInfo
	{
		int					nIndex;								///< 스폰 인덱스
		u64	nRecentSpawnTime[MAX_SPAWN_COUNT];	///< 최근에 스폰된 시간
	};

	MQuestLevelStaticInfo		m_StaticInfo;					///< 퀘스트 시작하면 변하지 않는 정보
	MQuestLevelDynamicInfo		m_DynamicInfo;					///< 퀘스트 진행하면서 변하는 정보
	MQuestNPCQueue				m_NPCQueue;						///< 나오는 NPC 오브젝트 큐

	MQuestLevelSpawnInfo		m_SpawnInfos[MNST_END];			///< NPC 스폰지점 정보

	bool InitSectors();
	bool InitNPCs();											
	void InitStaticInfo();
	void InitCurrSector();

public:
	MQuestLevel();						///< 생성자
	~MQuestLevel();						///< 소멸자

	/// 시나리오 ID를 기반으로 월드레벨 초기화
	/// @param nScenarioID			시나리오 ID
	/// @param nDice				주사위 굴림
	void Init(int nScenarioID, int nDice);
	/// 설정된 월드레벨을 바탕으로 클라이언트에 보내줄 전송데이타를 만든다.
	void Make_MTDQuestGameInfo(MTD_QuestGameInfo* pout);
	/// 맵섹터 수 반환
	/// @return		섹터 수
	int GetMapSectorCount();
	/// 현재 섹터 인덱스 반환
	/// @return		현재 섹터 인덱스
	int GetCurrSectorIndex();
	/// 다음 섹터로 이동한다.
	/// @return		성공/실패 여부
	bool MoveToNextSector();	
	/// 새로 태어날 NPC의 위치를 추천한다.
	/// @param nSpawnType		NPC의 스폰타입
	/// @param nNowTime			현재 시간
	int GetRecommendedSpawnPosition(MQuestNPCSpawnType nSpawnType, u64 nNowTime);
	/// 지금 NPC가 스폰가능한지 확인한다.
	/// @param nSpawnType		NPC의 스폰타입
	/// @param nNowTime			현재 시간
	bool IsEnableSpawnNow(MQuestNPCSpawnType nSpawnType, u64 nNowTime);
	/// 현재 섹터에서 스폰 지역 개수를 구한다.
	/// @param nSpawnType		NPC의 스폰타입
	int GetSpawnPositionCount(MQuestNPCSpawnType nSpawnType);

	/// 퀘스트 아이템이 생성될 경우 호출된다.
	/// @nItemID				퀘스트 아이템 ID
	/// @nRentPeriodHour		일반 아이템일 경우 사용 기간
	void OnItemCreated(u32	nItemID, int nRentPeriodHour);
	/// 플레이어가 퀘스트 아이템 먹었을 경우 호출된다.
	/// @nItemID				퀘스트 아이템 ID
	bool OnItemObtained( MMatchObject* pPlayer, u32	nItemID);		

	MQuestNPCQueue* GetNPCQueue()				{ return &m_NPCQueue; }
	MQuestLevelStaticInfo* GetStaticInfo()		{ return &m_StaticInfo; }
	MQuestLevelDynamicInfo* GetDynamicInfo()	{ return &m_DynamicInfo; }
};






#endif