#ifndef _MBASEQUEST_H
#define _MBASEQUEST_H

// 이곳에는 퀘스트관련 클라이언트와 서버가 공통으로 사용하는 것을 넣도록 한다.
//////////////////////////////////////////////////////////////////////////////

#include "MQuestNPC.h"
#include "MQuestMap.h"
#include "MSurvivalMap.h"
#include "MQuestDropTable.h"

#include <map>
using namespace std;


struct MQuestNPCInfo;

/// 퀘스트 월드레벨에서 사용하는 섹터 노드
struct MQuestLevelSectorNode
{
	int		nSectorID;
	int		nNextLinkIndex;

	// 여기에 추가정보 들어갈듯
};

////////////////////////////////////////////////////////////////////////////////////////////
/// 서버와 클라이언트 퀘스트 최고 관장클래스의 공통 부모 클래스
class MBaseQuest
{
private:
	bool m_bCreated;
protected:
	
	MQuestMapCatalogue			m_MapCatalogue;								///< 맵 정보
	MQuestNPCCatalogue			m_NPCCatalogue;								///< NPC 정보
	MSurvivalMapCatalogue		m_SurvivalMapCatalogue;						///< 서바이벌 모드용 맵 정보
	MQuestDropTable				m_DropTable;								///< 드롭 테이블 정보
	virtual bool OnCreate();												///< Create()호출시 불리는 함수
	virtual void OnDestroy();												///< Destroy()호출시 불리는 함수

	void ProcessNPCDropTableMatching();		// npc.xml의 Droptable을 매칭시킨다.
public:
	MBaseQuest();															///< 생성자
	virtual ~MBaseQuest();													///< 소멸자
	inline MQuestMapSectorInfo*		GetSectorInfo(int nSectorID);			///< 섹터 정보 반환
	inline MQuestNPCInfo*			GetNPCInfo(MQUEST_NPC nNPC);			///< NPC 정보 반환
	inline MQuestNPCInfo*			GetNPCPageInfo( int nPage);				///< NPC 정보 반환
	inline MSurvivalMapInfo*		GetSurvivalMapInfo(MSURVIVAL_MAP nMap);	///< 서바이벌 모드용 맵 정보 반환
	inline MQuestDropTable*			GetDropTable();							///< 퀘스트 아이템 드롭 테이블 정보 반환
	bool Create();															///< 초기화
	int GetNumOfPage( void)			{ return (int)m_NPCCatalogue.size(); }
	void Destroy();															///< 해제

	MQuestNPCInfo* GetNPCIndexInfo( int nMonsterBibleIndex )
	{
		return m_NPCCatalogue.GetIndexInfo( nMonsterBibleIndex );
	}

	inline MQuestMapCatalogue* GetMapCatalogue();
	inline MQuestNPCCatalogue* GetNPCCatalogue();
};





// inline functions //////////////////////////////////////////////////////////////////////////
inline MQuestMapSectorInfo* MBaseQuest::GetSectorInfo(int nSectorID)
{
	return m_MapCatalogue.GetSectorInfo(nSectorID);
}

inline MQuestNPCInfo* MBaseQuest::GetNPCInfo(MQUEST_NPC nNPC)
{
	return m_NPCCatalogue.GetInfo(nNPC);
}

inline MQuestNPCInfo* MBaseQuest::GetNPCPageInfo( int nPage)
{
	return m_NPCCatalogue.GetPageInfo(nPage);
}

inline MSurvivalMapInfo* MBaseQuest::GetSurvivalMapInfo(MSURVIVAL_MAP nMap)
{
	return m_SurvivalMapCatalogue.GetInfo(nMap);
}

inline MQuestDropTable* MBaseQuest::GetDropTable()
{
	return &m_DropTable;
}

inline MQuestMapCatalogue* MBaseQuest::GetMapCatalogue()
{
	return &m_MapCatalogue;
}

inline MQuestNPCCatalogue* MBaseQuest::GetNPCCatalogue()
{
	return &m_NPCCatalogue;
}


#endif