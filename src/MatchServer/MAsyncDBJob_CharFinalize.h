#ifndef _MASYNCDBJOB_CHARFINALIZE_H
#define _MASYNCDBJOB_CHARFINALIZE_H

#include "MAsyncDBJob.h"

#include "MQuestItem.h"

class MAsyncDBJob_CharFinalize : public MAsyncJob {
protected:	// Input Argument
	int					m_nCID;
	u32	m_nPlayTime;
	int					m_nConnKillCount;
	int					m_nConnDeathCount;
	int					m_nConnXP;
	int					m_nXP;
	MQuestItemMap		m_QuestItemMap;
	MQuestMonsterBible	m_QuestMonster;
	bool				m_bIsRequestQItemUpdate;
	
protected:	// Output Result

public:
	MAsyncDBJob_CharFinalize()
		: MAsyncJob(MASYNCJOB_CHARFINALIZE), m_bIsRequestQItemUpdate( false )
	{

	}
	virtual ~MAsyncDBJob_CharFinalize()
	{
		m_QuestItemMap.Clear();
	}

	bool Input( int	nCID, 
				u32 nPlayTime, 
				int nConnKillCount, 
				int nConnDeathCount, 
				int nConnXP, 
				int nXP,
				MQuestItemMap& rfQuestItemMap,
				MQuestMonsterBible& rfQuestMonster,
				const bool bIsRequestQItemUpdate );


	virtual void Run(void* pContext);
};





#endif