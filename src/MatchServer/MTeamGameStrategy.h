#ifndef _MTEAMGAMESTRATEGY_H
#define _MTEAMGAMESTRATEGY_H

#include "MMatchGlobal.h"
#include <vector>
using namespace std;

class MMatchObject;
class MLadderGroup;
class MMatchStage;

class MLadderGameStrategy;
class MClanGameStrategy;
struct MMatchLadderTeamInfo;

class MBaseTeamGameStrategy
{
protected:
	MBaseTeamGameStrategy() { }
	virtual ~MBaseTeamGameStrategy() { }
public:
	/// 도전 가능한지 체크한다.
	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount) = 0;

	/// 도전하자고 다른사람들한테 제안할 수 있는지 체크한다.
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount) = 0;
	/// 새로운 LadderGroup ID를 생성해서 반환한다.
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount) = 0;

	/// LadderGroup의 필요한 정보를 세팅한다. ID빼고..
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount) = 0;

	/// Stage에서 필요한 LadderInfo를 세팅한다.
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup) = 0;

	/// 게임이 끝났을때 결과를 DB 저장한다.
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo) = 0;

	virtual int GetRandomMap(int nTeamMember) = 0;

	/// 서버모드에 따라 적당한 자식 클래스를 반환한다. MSM_LADDER, MSM_CLAN만 가능
	static MBaseTeamGameStrategy* GetInstance(MMatchServerMode nServerMode);
};


class MLadderGameStrategy : public MBaseTeamGameStrategy
{
protected:
	MLadderGameStrategy() { }
public:
	static MLadderGameStrategy* GetInstance()
	{
		static MLadderGameStrategy m_stInstance;
		return &m_stInstance;
	}
	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount);
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount);
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount) { }
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup);
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo);
	virtual int GetRandomMap(int nTeamMember);
};


class MClanGameStrategy : public MBaseTeamGameStrategy
{
protected:
	MClanGameStrategy();
	vector<int>		m_RandomMapVec[MLADDERTYPE_MAX];
public:
	static MClanGameStrategy* GetInstance()
	{
		static MClanGameStrategy m_stInstance;
		return &m_stInstance;
	}

	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount);
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount);
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup);
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo);
	virtual int GetRandomMap(int nTeamMember);
};







#endif