#ifndef _MQUEST_PLAYER_H
#define _MQUEST_PLAYER_H


#include "MMatchNPCObject.h"
#include "MQuestItem.h"

#include <map>
using std::map;

struct RewardZItemInfo
{
	unsigned int		nItemID;
	int					nRentPeriodHour;
};

class MQuestRewardZItemList : public std::list<RewardZItemInfo>
{
};

/// 퀘스트 룰에서 쓰이는 플레이 정보
struct MQuestPlayerInfo
{
	// NPC Control 관련 /////////
	MMatchObject*		pObject;					///< Object 정보
	u32	nNPCControlCheckSum;		///< NPC 조종 체크섬
	MMatchNPCObjectMap	NPCObjects;					///< 조종중인 NPC
	bool				bEnableNPCControl;			///< NPC Control이 가능한지 여부

	/// NPC 관리 점수
	/// - 스코어가 낮을수록 우선순위가 높아짐
	int GetNPCControlScore()						
	{
		// 지금은 그냥 조종하는 NPC 개수
		return (int)(NPCObjects.size());
	}


	// 퀘스트 룰 관련 ///////////
	bool				bMovedtoNewSector;			///< 다음 섹터로 이동했는지 여부


	// 서바이벌 룰 관련 /////////



	// 보상 관련 ////////////////
	int						nQL;						///< QL
	int						nDeathCount;				///< 죽은 회수
	int						nUsedPageSacriItemCount;	///< 기본 희생 아이템 사용 개수(페이지)
	int						nUsedExtraSacriItemCount;	///< 추가 희생 아이템 사용 개수
	int						nXP;						///< 얻은 XP
	int						nBP;						///< 얻은 BP

	MQuestItemMap			RewardQuestItemMap;			///< 얻은 퀘스트 아이템
	MQuestRewardZItemList	RewardZItemList;


	// Log관련 ////////////////// - by 추교성.
	// char				szName[ 24 ];


	/// 초기화
	/// @param pObj		플레이어 오브젝트 정보
	/// @param a_nQL	플레이어 퀘스트 레벨
	void Init(MMatchObject* pObj, int a_nQL)
	{
		pObject = pObj;
		bEnableNPCControl = true;
		nNPCControlCheckSum = 0;
		NPCObjects.clear();
		bMovedtoNewSector = true;

		nQL = a_nQL;
		nDeathCount = 0;
		nUsedPageSacriItemCount = 0;
		nUsedExtraSacriItemCount = 0;
		nXP = 0;
		nBP = 0;

		RewardQuestItemMap.Clear();
		RewardZItemList.clear();
	}

	/// 생성자
	MQuestPlayerInfo() : nXP(0), nBP(0)
	{
		
	}
};

/// 퀘스트룰의 플레이어 오브젝트 관리자
class MQuestPlayerManager : public map<MUID, MQuestPlayerInfo*>
{
private:
	MMatchStage* m_pStage;
	void AddPlayer(MUID& uidPlayer);
public:
	MQuestPlayerManager();										///< 생성자
	~MQuestPlayerManager();										///< 소멸자
	void Create(MMatchStage* pStage);							///< 초기화
	void Destroy();												///< 해제
	void DelPlayer(MUID& uidPlayer);							///< 플레이어 삭제
	void Clear();												///< 초기화
	MQuestPlayerInfo* GetPlayerInfo(const MUID& uidPlayer);		///< 플레이어 정보 반환
};

#endif