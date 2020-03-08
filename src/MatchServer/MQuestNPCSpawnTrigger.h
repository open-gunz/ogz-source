#ifndef _MQUESTNPCSPAWNTRIGGER_H
#define _MQUESTNPCSPAWNTRIGGER_H

#include "MBaseQuest.h"


struct SpawnTriggerNPCInfoNode
{
	MQUEST_NPC	nNPCID;
	float		fRate;
};

struct SpawnTriggerInfo
{
	int				nSpawnNPCCount;			// 1회 스폰시 스폰될 NPC수
	unsigned int	nSpawnTickTime;			// 스폰 틱 타임

	// 조건
	int				nCurrMinNPCCount;		// 이값이하일때 스폰한다.
	int				nCurrMaxNPCCount;		// 이값이상일때 스폰한다.
};

class MQuestNPCSpawnTrigger
{
private:
	vector<SpawnTriggerNPCInfoNode>		m_NPCInfo;
	SpawnTriggerInfo					m_Info;
	u64									m_nLastTime;
	float								m_fRateSum;
	vector<MQUEST_NPC>					m_NPCQueue;
	void MakeSpawnNPCs();
	MQUEST_NPC GetRandomNPC();
public:
	MQuestNPCSpawnTrigger();
	~MQuestNPCSpawnTrigger();
	void BuildNPCInfo(SpawnTriggerNPCInfoNode& NPCInfo);
	void BuildCondition(SpawnTriggerInfo& Info);

	bool CheckSpawnEnable(int nCurrNPCCount);
	void Clear();
	vector<MQUEST_NPC>& GetQueue() { return m_NPCQueue; }
};



#endif