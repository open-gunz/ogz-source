#ifndef _MQUEST_SCENARIO_H
#define _MQUEST_SCENARIO_H


#include "MQuestConst.h"

struct MQuestScenarioInfoMapJaco
{
	MQUEST_NPC	nNPCID;
	float		fRate;
};


struct MQuestScenarioInfoMaps
{
	int										nKeySectorID;			///< 최종 섹터 ID
	int										nKeyNPCID;				///< 키 NPC ID
	bool									bKeyNPCIsBoss;			///< 키 NPC가 보스인지 여부
	vector<int>								vecNPCSetArray;			///< NPC Set Array

	// 보스방에서 쓸 정보
	int										nJacoCount;				///< 1회 스폰시 스폰될 졸병수
	unsigned int							nJacoSpawnTickTime;		///< 졸병 스폰 틱 타임
	int										nJacoMinNPCCount;		///< 이값이하일때는 졸병이 스폰하지 않는다.
	int										nJacoMaxNPCCount;		///< 이값이하일때는 졸병이 스폰하지 않는다.
	vector<MQuestScenarioInfoMapJaco>		vecJacoArray;			///< 보스방에서 나올 졸병들

	MQuestScenarioInfoMaps()
	{
		nKeySectorID = 0;
		nKeyNPCID = 0;
		bKeyNPCIsBoss = false;
		nJacoCount = 0;
		nJacoSpawnTickTime = 9999999;
		nJacoMinNPCCount = 0;
		nJacoMaxNPCCount = 0;
	}
};

/// 퀘스트 시나리오 정보
struct MQuestScenarioInfo
{
	int				nID;										///< 시나리오 ID
	char			szTitle[64];								///< 시나리오 이름
	int				nQL;										///< 요구 퀘스트 레벨
	float			fDC;										///< 난이도 계수(DC)
	int				nResSacriItemCount;							///< 시나리오를 위한 희생 아이템 개수
	unsigned int	nResSacriItemID[MAX_SCENARIO_SACRI_ITEM];	///< 시나리오를 위한 희생 아이템
	int				nMapSet;									///< 맵셋
	bool			bSpecialScenario;							///< 특별시나리오인지 여부

	int				nXPReward;									///< XP 보상치
	int				nBPReward;									///< BP 보상치
	int				nRewardItemCount;							///< 특별 아이템 개수
	int				nRewardItemID[MAX_SCENARIO_REWARD_ITEM];	///< 특별 아이템 보상
	float			fRewardItemRate[MAX_SCENARIO_REWARD_ITEM];	///< 특별 아이템 보상 확률
	int				nSectorXP;									///< 섹터별 보너스 XP 보상치
	int				nSectorBP;									///< 섹터별 보너스 BP 보상치

	MQuestScenarioInfoMaps		Maps[SCENARIO_STANDARD_DICE_SIDES];

	/// 생성자
	MQuestScenarioInfo()
	{
		nID = -1;
		szTitle[0] = 0;
		nQL = 0;
		fDC = 0.0f;
		nResSacriItemCount = 0;
		memset(nResSacriItemID, 0, sizeof(nResSacriItemID));
		nMapSet = 0;
		nXPReward = 0;
		nBPReward = 0;
		nRewardItemCount = 0;
		memset(fRewardItemRate, 0, sizeof(fRewardItemRate));
		bSpecialScenario = false;

		for (int i = 0; i < SCENARIO_STANDARD_DICE_SIDES; i++)
		{
			Maps[i].nKeySectorID = 0;
			Maps[i].nKeyNPCID = 0;
		}

		nSectorXP = -1;
		nSectorBP = -1;
	}

	/// 섹터 수 반환
	int GetSectorCount(int nDice)
	{
		return (int)Maps[nDice].vecNPCSetArray.size();
	}
};

/// 시나리오 정보 관리자
class MQuestScenarioCatalogue : public map<int, MQuestScenarioInfo*>
{
private:
	// 멤버 변수
	int		m_nDefaultStandardScenarioID;
	// 함수
	void Clear();
	void Insert(MQuestScenarioInfo* pScenarioInfo);
	void ParseSpecialScenario(MXmlElement& element);
	void ParseStandardScenario(MXmlElement& element);
	void ParseNPCSetArray(MXmlElement& element, vector<int>& vec);
	void ParseJaco(MXmlElement& element, MQuestScenarioInfoMaps* pMap);
	void ParseRewardItem(MXmlElement& element, MQuestScenarioInfo* pScenarioInfo);
	void ParseSacriItem(MXmlElement& element, MQuestScenarioInfo* pScenarioInfo);
	void ParseMap(MXmlElement& element, MQuestScenarioInfo* pScenarioInfo);
	int CalcStandardScenarioID(int nMapsetID, int nQL);
public:
	MQuestScenarioCatalogue();											///< 생성자
	~MQuestScenarioCatalogue();											///< 소멸자

	bool ReadXml(const char* szFileName);								///< xml로부터 npc정보를 읽는다.
	bool ReadXml(MZFileSystem* pFileSystem,const char* szFileName);		///< xml로부터 npc정보를 읽는다.
	

	MQuestScenarioInfo* GetInfo(int nScenarioID);						///< 시나리오 정보 반환
	/// 정규 시나리오 반환
	/// @param nQL				퀘스트 레벨
	/// @param nDice			주사위 굴림
	int GetStandardScenarioID(int nMapsetID, int nQL);

	/// 특별 시나리오 검색
	/// @param nMapsetID		맵셋
	/// @param nQL				퀘스트 레벨
	bool FindSpecialScenarioID(int nMapsetID, int nPlayerQL, unsigned int* SacriQItemIDs, unsigned int* outScenarioID);

	unsigned int MakeScenarioID(int nMapsetID, int nPlayerQL, unsigned int* SacriQItemIDs);


	const int GetDefaultStandardScenarioID() { return m_nDefaultStandardScenarioID; }
};


#endif