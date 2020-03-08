#pragma once

#include "MMatchRule.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchNPCObject.h"
#include "MMatchQuestRound.h"
#include "MSacrificeQItemTable.h"
#include "MQuestItem.h"
#include "MMatchQuestGameLog.h"
#include "MQuestNPCSpawnTrigger.h"
#include "MQuestLevel.h"
#include "MMatchGameType.h"

typedef std::pair< MUID, u32 > SacrificeSlot;

class MQuestSacrificeSlot
{
public :
	MQuestSacrificeSlot() 
	{
		Release();
	}

	~MQuestSacrificeSlot()
	{
	}

	MUID&				GetOwnerUID()	{ return m_SacrificeSlot.first; }
	u32	GetItemID()		{ return m_SacrificeSlot.second; }

	bool IsEmpty() 
	{
		if( (MUID(0, 0) == m_SacrificeSlot.first) && (0 == m_SacrificeSlot.second) )
			return true;
		return false;
	}


	void SetOwnerUID( const MUID& uidItemOwner )		{ m_SacrificeSlot.first = uidItemOwner; }
	void SetItemID( const u32 nItemID )	{ m_SacrificeSlot.second = nItemID; }
	void SetAll( const MUID& uidItemOwner, const u32 nItemID )
	{
		SetOwnerUID( uidItemOwner );
		SetItemID( nItemID );
	}


	bool IsOwner( const MUID& uidRequester, const u32 nItemID )
	{
		if( (m_SacrificeSlot.first == uidRequester) && (m_SacrificeSlot.second == nItemID) )
			return true;
		return false;
	}

	void Release()
	{
		m_SacrificeSlot.first	= MUID( 0, 0 );
		m_SacrificeSlot.second	= 0;
	}

private :
	SacrificeSlot	m_SacrificeSlot;
};

class MMatchRuleQuest : public MMatchRuleBaseQuest {
private:
	struct MQuestStageGameInfo
	{
		int				nQL;
		int				nPlayerQL;
		int				nMapsetID;
		unsigned int	nScenarioID;
	};

	enum COMBAT_PLAY_RESULT
	{
		CPR_PLAYING = 0,
		CPR_COMPLETE,
		CPR_FAILED
	};

	u64	m_nPrepareStartTime;
	u64	m_nCombatStartTime;
	u64	m_nQuestCompleteTime;

	MQuestSacrificeSlot				m_SacrificeSlot[ MAX_SACRIFICE_SLOT_COUNT ];
	int								m_nPlayerCount;
	MMatchQuestGameLogInfoManager	m_QuestGameLogInfoMgr;

	MQuestStageGameInfo				m_StageGameInfo;

	void ClearQuestLevel();
	void MakeStageGameInfo();
	void InitJacoSpawnTrigger();
	void MakeNPCnSpawn(MQUEST_NPC nNPCID, bool bAddQuestDropItem);
protected:
	MQuestLevel*			m_pQuestLevel;			///< 퀘스트 월드 레벨
	MQuestNPCSpawnTrigger	m_JacoSpawnTrigger;		///< 보스방일 경우 자코 매니져
	MQuestCombatState		m_nCombatState;			///< 섹터내 전투 상태

	virtual void ProcessNPCSpawn();				///< NPC 스폰작업
	virtual bool CheckNPCSpawnEnable();			///< NPC가 스폰 가능한지 여부
	virtual void RouteGameInfo();				///< 클라이언트에 게임 정보 보내준다.
	virtual void RouteStageGameInfo();			///< 대기중 스테이지에서 바뀐 게임 정보를 보내준다.
	virtual void RouteCompleted();				///< 퀘스트 성공 메시지를 보낸다. - 리워드까지 함께 보낸다.
	virtual void RouteFailed();					///< 퀘스트 실패 메시지 보낸다.
	virtual void OnCompleted();					///< 퀘스트 성공시 호출된다.
	virtual void OnFailed();					///< 퀘스트 실패시 호출된다.
	virtual void DistributeReward();			///< 퀘스트 성공시 리워드 배분
	
	/// 섹터 라운드 시작되었다고 메세지 보낸다.
	void RouteMapSectorStart();
	/// 해당 플레이어가 포탈로 이동했다고 메세지 보낸다.
	/// @param uidPlayer 이동한 플레이어 UID
	void RouteMovetoPortal(const MUID& uidPlayer);
	/// 해당 플레이어가 포탈로 이동이 완료되었다고 메세지 보낸다.
	/// @param uidPlayer 이동한 플레이어 UID
	void RouteReadyToNewSector(const MUID& uidPlayer);
	/// 해당 퀘스트 아이템을 먹었다고 메세지 보낸다.
	/// @param nQuestItemID  퀘스트 아이템 ID
	void RouteObtainQuestItem(u32 nQuestItemID);
	/// 해당 일반 아이템을 먹었다고 메세지 보낸다.
	/// @param nItemID  일반 아이템 ID
	void RouteObtainZItem(u32 nItemID);
	/// 섹터 경험치 라우트
	void RouteSectorBonus(const MUID& uidPlayer, u32 nEXPValue);
	/// 섹터 전투 상태 변화시 메세지 보낸다.
	void RouteCombatState();
	/// 퀘스트 레벨 생성
	bool MakeQuestLevel();
	/// 섹터 전투 처리 작업
	/// - 나중에 일련의 Combat 상태 관리는 Survival만들때 MMatchRuleBaseQuest로 옮겨져야 한다.
	void CombatProcess();
	/// 다음 섹터로 이동
	void MoveToNextSector();			
	/// 섹터 전투 상태 변환
	void SetCombatState(MQuestCombatState nState);
	/// 다음 섹터로 이동완료되었는지 체크
	bool CheckReadytoNewSector();
	/// 섹터 라운드가 끝났는지 체크한다.
	COMBAT_PLAY_RESULT CheckCombatPlay();
	/// 퀘스트가 모두 끝났는지 체크한다.
	bool CheckQuestCompleted();
	/// 퀘스트 Complete하고 나서 아이템 먹을 수 있는 지연시간 계산
	bool CheckQuestCompleteDelayTime();
	/// 섹터 클리어시 호출된다.
	void OnSectorCompleted();
	/// 섹터 전투 처리 작업
	void ProcessCombatPlay();

	/// 해당 전투 상태 처음 시작할때
	void OnBeginCombatState(MQuestCombatState nState);
	/// 해당 전투 상태 끝났을때
	void OnEndCombatState(MQuestCombatState nState);

	///< 게임종료후 보상 분배
	void MakeRewardList();
	///< 경험치와 바운티를 배분.
	void DistributeXPnBP( MQuestPlayerInfo* pPlayerInfo, const int nRewardXP, const int nRewardBP, const int nScenarioQL );	
	///< 퀘스트에서 얻은 퀘스트 아이템 분배.		
	bool DistributeQItem( MQuestPlayerInfo* pPlayerInfo, void** ppoutSimpleQuestItemBlob);
	///< 퀘스트에서 얻은 일반 아이템 분배.		
	bool DistributeZItem( MQuestPlayerInfo* pPlayerInfo, void** ppoutQuestRewardZItemBlob);
protected:
	virtual void OnBegin();								///< 전체 게임 시작시 호출
	virtual void OnEnd();								///< 전체 게임 종료시 호출
	virtual bool OnRun();								///< 게임틱시 호출
	virtual void OnCommand(MCommand* pCommand);			///< 퀘스트에서만 사용하는 커맨드 처리
	virtual bool OnCheckRoundFinish();					///< 라운드가 끝났는지 체크
public:
	MMatchRuleQuest(MMatchStage* pStage);				///< 생성자
	virtual ~MMatchRuleQuest();							///< 소멸자

	void RefreshStageGameInfo();

	/// 플레이어 죽었을 때 호출
	/// @param uidVictim		죽은 플레이어 UID
	virtual void OnRequestPlayerDead(const MUID& uidVictim);
	/// 월드 아이템을 먹었을 경우 호출된다.
	/// @param pObj				플레이어 오브젝트
	/// @param nItemID			월드 아이템 ID
	/// @param nQuestItemID		퀘스트 아이템 ID
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues);

	void OnRequestTestSectorClear();
	void OnRequestTestFinish();

	/// 플레이어가 포탈로 이동했을 경우 호출된다.
	/// @param uidPlayer			이동한 플레이어 UID
	void OnRequestMovetoPortal(const MUID& uidPlayer);
	/// 포탈로 이동하고 나서 이동이 완료되었을 경우 호출된다.
	/// @param uidPlayer			플레이어 UID
	void OnReadyToNewSector(const MUID& uidPlayer);

	virtual void OnRequestDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const u32 nItemID );
	virtual void OnResponseDropSacrificeItemOnSlot( const MUID& uidSender, const int nSlotIndex, const u32 nItemID );
	virtual void OnRequestCallbackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const u32 nItemID );
    virtual void OnResponseCallBackSacrificeItem( const MUID& uidSender, const int nSlotIndex, const u32 nItemID );
	virtual void OnRequestQL( const MUID& uidSender );
	virtual void OnResponseQL_ToStage( const MUID& uidStage );
	virtual void OnRequestSacrificeSlotInfo( const MUID& uidSender );
	virtual void OnResponseSacrificeSlotInfoToListener( const MUID& uidSender );
	virtual void OnResponseSacrificeSlotInfoToStage( const MUID& uidStage );
	virtual void OnChangeCondition();

	virtual bool							PrepareStart();
	virtual bool							IsSacrificeItemDuplicated( const MUID& uidSender, const int nSlotIndex, const u32 nItemID );
	virtual void							PreProcessLeaveStage( const MUID& uidLeaverUID );
	virtual void							DestroyAllSlot();
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_QUEST; }


	void InsertNoParamQItemToPlayer( MMatchObject* pPlayer, MQuestItem* pQItem );
	
	void PostInsertQuestGameLogAsyncJob();

	// 게임이 시작할때 필요한 정보 수집.
	void CollectStartingQuestGameLogInfo();
	// 게임이 끝나고 필요한 정보 수집.
	void CollectEndQuestGameLogInfo();

	void RouteRewardCommandToStage( MMatchObject* pPlayer, const int nRewardXP, const int nRewardBP, void* pSimpleQuestItemBlob, void* pSimpleZItemBlob );
	
private :
	int CalcuOwnerQItemCount( const MUID& uidPlayer, const u32 nItemID );
};