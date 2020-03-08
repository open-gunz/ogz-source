#ifndef _MMATCH_QUEST_ROUND_H
#define _MMATCH_QUEST_ROUND_H


class MMatchQuestRound
{
private:
	int		m_nRound;
public:
	MMatchQuestRound();
	~MMatchQuestRound();
	
	void Increase();
	void Reset();

	int ClearConditionNPCCount(int nPlayerCount);
	int SpawnTime();
	int MaxCurrNPCCount(int nPlayerCount);
	MQUEST_NPC RandomNPC();
	int GetRandomSpawnPosIndex();

	int GetRound()		{ return m_nRound; }
};




#endif