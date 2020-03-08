#ifndef _ZMODULE_QUESTSTATUS_H
#define _ZMODULE_QUESTSTATUS_H


#include "ZModule.h"
#include "ZModuleID.h"


// 퀘스트에서 사용하는 플레이어 정보
class ZModule_QuestStatus : public ZModule {
private:
	int				m_nKills;		// NPC 킬수
public:
	DECLARE_ID(ZMID_QUESTSTATUS)
	ZModule_QuestStatus();
	virtual ~ZModule_QuestStatus();

	inline void AddKills();
	inline int GetKills();
};


// inline functions /////////////////////////////////////////////////////////////
inline void ZModule_QuestStatus::AddKills()
{
	m_nKills++;
}

inline int ZModule_QuestStatus::GetKills()
{
	return m_nKills;
}

#endif