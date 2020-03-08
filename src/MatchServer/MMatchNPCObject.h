#ifndef _MMATCHNPCOBJECT_H
#define _MMATCHNPCOBJECT_H


#include "MVector3.h"
#include "MBaseQuest.h"
#include "MUID.h"
#include "MQuestDropTable.h"
#include <map>
using namespace std;

class MMatchStage;
struct MQuestPlayerInfo;

/// NPC 오브젝트의 플래그 - 아직까진 별다른데 사용하지 않는다.
enum NPCOBJECT_FLAG
{
	NOF_NONE			= 0,

};


/// NPC 오브젝트
class MMatchNPCObject
{
private:
	MUID				m_UID;						///< ID
	MQUEST_NPC			m_nType;					///< NPC타입
	MUID				m_uidController;			///< 조종자
	MVector3			m_Pos;						///< 위치
	u32	m_nFlags;					///< 플래그 모음
	MQuestDropItem		m_DropItem;					///< 가지고 있는 아이템 - 없을 경우 nDropItemType가 QDIT_NA이다.

public:
	/// 생성자
	/// @param uid		NPC UID
	/// @param nType	NPC 종류
	/// @param nFlags	플래스
	MMatchNPCObject(MUID& uid, MQUEST_NPC nType, u32 nFlags=0);
	/// 소멸자
	~MMatchNPCObject() { }
	/// NPC 조종을 플레이어에게 할당한다.
	/// @param uidPlayer	할당할 플레이어 UID
	void AssignControl(MUID& uidPlayer);		
	/// NPC 조종자 해제
	void ReleaseControl();
	/// Drop할 아이템을 설정한다.
	/// @param pDropItem	아이템 정보
	void SetDropItem(MQuestDropItem* pDropItem);

	// gets
	MUID GetUID()					{ return m_UID; }				///< NPC UID 반환
	MQUEST_NPC	GetType()			{ return m_nType; }				///< NPC 종류 반환
	MUID& GetController()			{ return m_uidController; }		///< NPC 조종자(플레이어) UID 반환
	MQuestDropItem* GetDropItem()	{ return &m_DropItem; }			///< 드롭 아이템 정보 반환

	inline void SetFlag(unsigned int nFlag, bool bValue);			///< 플래그 설정
	inline bool CheckFlag(unsigned int nFlag);						///< 플래그 체크
	inline void SetFlags(unsigned int nFlags);						///< 플래그 설정
	inline u32 GetFlags();								///< 플래그 반환
	inline bool HasDropItem();										///< 드롭 아이템을 가지고 있는지 반환

};

typedef map<MUID, MMatchNPCObject*>		MMatchNPCObjectMap;


class MQuestPlayerManager;

/// NPC 오브젝트 관리자
class MMatchNPCManager
{
private:
	// var
	MMatchStage*					m_pStage;
	MQuestPlayerManager*			m_pPlayerManager;
	MMatchNPCObjectMap				m_NPCObjectMap;

	u32				m_nLastSpawnTime;		// for test

	int								m_nNPCCount[MNST_END];		// 스폰타입별 살아있는 NPC수
	int								m_nBossCount;				// 살아있는 보스 수

	// func
	MUID NewUID();
	bool AssignControl(MUID& uidNPC, MUID& uidPlayer);
	bool Spawn(MUID& uidNPC, MUID& uidController, unsigned char nSpawnPositionIndex);
	void Clear();
	bool FindSuitableController(MUID& out, MQuestPlayerInfo* pSender);

	void SetNPCObjectToControllerInfo(MUID& uidChar, MMatchNPCObject* pNPCObject);
	void DelNPCObjectToControllerInfo(MUID& uidChar, MMatchNPCObject* pNPCObject);
public:
	bool BossDead{};

	/// 생성자
	MMatchNPCManager();
	/// 소멸자
	~MMatchNPCManager();
	/// 초기화
	/// @param pStage				스테이지 클래스
	/// @param pPlayerManager		퀘스트룰에서의 PlayerManager
	void Create(MMatchStage* pStage, MQuestPlayerManager* pPlayerManager);
	/// 해제
	void Destroy();
	/// 모든 NPC를 없앤다.
	void ClearNPC();
	/// NPC 오브젝트 생성
	/// @param nType					NPC 종류
	/// @param nSpawnPositionIndex		스폰 위치
	MMatchNPCObject* CreateNPCObject(MQUEST_NPC nType, unsigned char nSpawnPositionIndex);
	/// NPC 오브젝트 해제
	/// @param uidNPC					NPC UID
	/// @param outItem					NPC가 드롭하는 아이템 반환값
	bool DestroyNPCObject(MUID& uidNPC, MQuestDropItem& outItem);
	/// NPC 오브젝트 반환
	/// @param uidNPC					NPC UID
	MMatchNPCObject* GetNPCObject(MUID& uidNPC);
	/// 플레이어가 스테이지에서 나갈때 호출된다.
	/// @param uidPlayer				플레이어 UID
	void OnDelPlayer(const MUID& uidPlayer);
	/// 해당 플레이어가 해당 NPC를 조종하고 있는지 체크
	/// @param uidChar					플레이어 UID
	/// @param uidNPC					NPC UID
	bool IsControllersNPC(MUID& uidChar, MUID& uidNPC);
	/// NPC 오브젝트수 반환
	int GetNPCObjectCount();
	/// 해당 스폰타입의 NPC 오브젝트수 반환
	/// @param nSpawnType				NPC 스폰 타입
	int GetNPCObjectCount(MQuestNPCSpawnType nSpawnType);
	int GetBossCount() { return m_nBossCount; }

	void RemovePlayerControl(const MUID& uidPlayer);
};



// inlines //////////////////////////////////////////////////////////////////////////////////
inline void MMatchNPCObject::SetFlags(unsigned int nFlags)
{
	if (m_nFlags != nFlags)
	{
		m_nFlags = nFlags;
	}
}

inline void MMatchNPCObject::SetFlag(unsigned int nFlag, bool bValue)
{
	if (bValue) m_nFlags |= nFlag;
	else m_nFlags &= ~nFlag;
}

inline bool MMatchNPCObject::CheckFlag(unsigned int nFlag)
{
	return ((m_nFlags & nFlag) != 0);
}

inline u32 MMatchNPCObject::GetFlags()
{ 
	return m_nFlags; 
}

inline int MMatchNPCManager::GetNPCObjectCount()
{
	return (int)m_NPCObjectMap.size();
}

inline bool MMatchNPCObject::HasDropItem()
{
	return (m_DropItem.nDropItemType != QDIT_NA);
}


inline int MMatchNPCManager::GetNPCObjectCount(MQuestNPCSpawnType nSpawnType)
{
	return m_nNPCCount[nSpawnType];
}


#endif