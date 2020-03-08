#include "stdafx.h"
#include "MMatchQuestRound.h"
#include "MMath.h"

//////////////
class MNPCSpawnDice
{
private:
	struct SpawnDiceInfo
	{
		MQUEST_NPC		nNPC;
		int				nWeight;
	};
	vector<SpawnDiceInfo>		m_InfoVector;
	int							m_nTotalWeight;
public:
	MNPCSpawnDice() : m_nTotalWeight(0)	{}
	void Add(MQUEST_NPC npc, int weight)
	{
		SpawnDiceInfo info;
		info.nNPC = npc;
		info.nWeight = weight;
		m_nTotalWeight += weight;
		m_InfoVector.push_back(info);
	}
	MQUEST_NPC Roll()
	{
		if (m_nTotalWeight > 0)
		{
			int nWeight = 0;
			int nRand = RandomNumber(0, m_nTotalWeight-1);

			for (int i = 0; i < (int)m_InfoVector.size(); i++)
			{
				SpawnDiceInfo info = m_InfoVector[i];
				nWeight += info.nWeight;

				if (nRand < nWeight) return info.nNPC;
			}
		}

		return NPC_GOBLIN;
	}
};
///////////////////


MMatchQuestRound::MMatchQuestRound() : m_nRound(1)
{

}

MMatchQuestRound::~MMatchQuestRound()
{


}

void MMatchQuestRound::Increase()
{
	m_nRound++;
}

void MMatchQuestRound::Reset()
{
	m_nRound = 1;
}

int MMatchQuestRound::ClearConditionNPCCount(int nPlayerCount)
{
	if (m_nRound < 4)
	{
		return (4 * nPlayerCount);
	}
	if (m_nRound < 8)
	{
		return (8 * nPlayerCount);
	}
	if (m_nRound < 12)
	{
		return (12 * nPlayerCount);
	}
	if (m_nRound < 16)
	{
		return (16 * nPlayerCount);
	}
	if (m_nRound < 20)
	{
		return (20 * nPlayerCount);
	}

	return (4*nPlayerCount*m_nRound);
}

int MMatchQuestRound::SpawnTime()
{
	if (m_nRound < 3)
	{
		return RandomNumber(500, 1000);
	}
	else if (m_nRound < 5)
	{
		return RandomNumber(500, 1000);
	}
	else if (m_nRound < 7)
	{
		return RandomNumber(500, 1000);
	}
	else if (m_nRound < 10)
	{
		return RandomNumber(500, 800);
	}
	else if (m_nRound < 15)
	{
		return RandomNumber(500, 700);
	}

	return 500;
}

int MMatchQuestRound::MaxCurrNPCCount(int nPlayerCount)
{
	float fRatio = 1.0f;
	if (m_nRound < 3)
	{
		return (int)(2*nPlayerCount*fRatio);
	}
	else if (m_nRound < 5)
	{
		return (int)(3*nPlayerCount*fRatio);
	}
	else if (m_nRound < 7)
	{
		return (int)(4*nPlayerCount*fRatio);
	}
	else if (m_nRound < 9)
	{
		return (int)(5*nPlayerCount*fRatio);
	}
	else if (m_nRound < 11)
	{
		return (int)(6*nPlayerCount*fRatio);
	}
	else if (m_nRound < 13)
	{
		return (int)(7*nPlayerCount*fRatio);
	}
	else if (m_nRound < 15)
	{
		return (int)(8*nPlayerCount*fRatio);
	}


	return (int)((m_nRound-5) * nPlayerCount *fRatio);
}

MQUEST_NPC MMatchQuestRound::RandomNPC()
{
	MNPCSpawnDice dice;
//	MQuestNPCInfo* pInfo;

	if (m_nRound < 10)
	{
		dice.Add(NPC_GOBLIN, 100);
	}
	else if (m_nRound < 15)
	{
		dice.Add(NPC_GOBLIN,			80);
		dice.Add(NPC_GOBLIN_GUNNER,		20);
	}
	else if (m_nRound < 20)
	{
		dice.Add(NPC_GOBLIN,			70);
		dice.Add(NPC_GOBLIN_GUNNER,		30);
	}
	else if (m_nRound < 25)
	{
		dice.Add(NPC_GOBLIN,			60);
		dice.Add(NPC_GOBLIN_GUNNER,		40);
		dice.Add(NPC_GOBLIN_COMMANDER,	5);
	}
	else if (m_nRound < 30)
	{
		dice.Add(NPC_GOBLIN,			60);
		dice.Add(NPC_GOBLIN_GUNNER,		35);
		dice.Add(NPC_GOBLIN_COMMANDER,	10);
	}
	else if (m_nRound < 35)
	{
		dice.Add(NPC_GOBLIN,			55);
		dice.Add(NPC_GOBLIN_GUNNER,		35);
		dice.Add(NPC_GOBLIN_COMMANDER,	15);
	}
	else if (m_nRound < 40)
	{
		dice.Add(NPC_GOBLIN,			50);
		dice.Add(NPC_GOBLIN_GUNNER,		35);
		dice.Add(NPC_GOBLIN_COMMANDER,	20);
	}
	else if (m_nRound < 50)
	{
		dice.Add(NPC_GOBLIN,			40);
		dice.Add(NPC_GOBLIN_GUNNER,		35);
		dice.Add(NPC_GOBLIN_COMMANDER,	25);
	}
	else
	{
		dice.Add(NPC_GOBLIN,			30);
		dice.Add(NPC_GOBLIN_GUNNER,		30);
		dice.Add(NPC_GOBLIN_COMMANDER,	40);
	}

	int nNPCType = (int)dice.Roll();

	return MQUEST_NPC(nNPCType);
}

int MMatchQuestRound::GetRandomSpawnPosIndex()
{
	int nPosIndex = RandomNumber(0, 26);
	return nPosIndex;
}