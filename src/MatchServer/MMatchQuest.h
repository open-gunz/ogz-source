#ifndef _MMATCHQUEST_H
#define _MMATCHQUEST_H

#include "MBaseQuest.h"
#include "MQuestScenario.h"


/// 서버용 퀘스트 최고 관장 클래스
class MMatchQuest : public MBaseQuest
{
protected:
	virtual bool OnCreate();				///< 초기화
	virtual void OnDestroy();				///< 해제
	

	MQuestNPCSetCatalogue		m_NPCSetCatalogue;						///< NPC Set 정보
	MQuestScenarioCatalogue		m_ScenarioCatalogue;					///< 시나리오 정보

public:
	MMatchQuest();														///< 생성자
	virtual ~MMatchQuest();												///< 소멸자

	inline MQuestNPCSetInfo* GetNPCSetInfo(int nID);					///< NPC Set 정보 반환
	inline MQuestNPCSetInfo* GetNPCSetInfo(const char* szName);			///< NPC Set 정보 반환
	inline MQuestScenarioCatalogue* GetScenarioCatalogue();				///< 시나리오 정보 반환
	inline MQuestScenarioInfo*		GetScenarioInfo(int nScenarioID);	///< 시나리오 정보 반환

};




inline MQuestNPCSetInfo* MMatchQuest::GetNPCSetInfo(int nID)
{
	return m_NPCSetCatalogue.GetInfo(nID);
}

inline MQuestNPCSetInfo* MMatchQuest::GetNPCSetInfo(const char* szName)
{
	return m_NPCSetCatalogue.GetInfo(szName);
}

inline MQuestScenarioCatalogue* MMatchQuest::GetScenarioCatalogue()
{
	return &m_ScenarioCatalogue;
}

inline MQuestScenarioInfo* MMatchQuest::GetScenarioInfo(int nScenarioID)
{
	return m_ScenarioCatalogue.GetInfo(nScenarioID);
}


#endif