#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchStage.h"
#include "MSharedCommandTable.h"
#include "MDebug.h"
#include "MMatchConfig.h"
#include "MTeamGameStrategy.h"
#include "MLadderGroup.h"
#include "MBlobArray.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleDeathMatch.h"
#include "MMatchRuleGladiator.h"
#include "MMatchRuleAssassinate.h"
#include "MMatchRuleTraining.h"
#include "MMatchRuleSurvival.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MMatchRuleSkillmap.h"
#include "MMatchRuleGunGame.h"
#include "MErrorTable.h"

MMatchStage::MMatchStage() : MovingWeaponMgr(*this), m_WorldItemManager(this)
{
	m_pRule = NULL;
	m_nIndex = 0;
	m_nStageType = MST_NORMAL;
	m_uidOwnerChannel = MUID(0,0);
	m_TeamBonus.bApplyTeamBonus = false;
	m_nAdminObjectCount = 0;
	memset(m_Teams, 0, sizeof(m_Teams));
}

bool MMatchStage::Create(const MUID& uid, const char* pszName, bool bPrivate, const char* pszPassword)
{
	if ((strlen(pszName) >= STAGENAME_LENGTH) || (strlen(pszPassword) >= STAGENAME_LENGTH)) return false;

	m_nStageType = MST_NORMAL;
	m_uidStage = uid;
	strcpy_safe(m_szStageName, pszName);
	strcpy_safe(m_szStagePassword, pszPassword);
	m_bPrivate = bPrivate;

	ChangeState(STAGE_STATE_STANDBY);
	ChangeRule(MMATCH_GAMETYPE_DEFAULT);

	SetAgentUID(MUID(0,0));
	SetAgentReady(false);

	m_nChecksum = 0;
	m_nLastChecksumTick = 0;
	m_nAdminObjectCount = 0;

	SetFirstMasterName("");

	return true;
}

void MMatchStage::Destroy()
{
	auto itor=GetObjBegin();
	while(itor!=GetObjEnd()) {
		MUID uidPlayer = (*itor).first;
		itor = RemoveObject(uidPlayer);
	}
	m_ObjUIDCaches.clear();

	if (m_pRule != NULL)
	{
		delete m_pRule;
		m_pRule = NULL;
	}
}

bool MMatchStage::IsChecksumUpdateTime(u64 nTick) const
{
	if (nTick - m_nLastChecksumTick > CYCLE_STAGE_UPDATECHECKSUM)
		return true;
	else
		return false;
}

void MMatchStage::UpdateChecksum(u64 nTick)
{
	m_nChecksum = (m_nIndex + 
		           GetState() + 
				   m_StageSetting.GetChecksum() + 
				   (u32)m_ObjUIDCaches.size());

	m_nLastChecksumTick = nTick;
}

void MMatchStage::UpdateStateTimer()
{
	m_nStateTimer = MMatchServer::GetInstance()->GetGlobalClockCount();
}

void MMatchStage::AddBanList(int nCID)
{
	if (CheckBanList(nCID))
		return;

	m_BanCIDList.push_back(nCID);
}

bool MMatchStage::CheckBanList(int nCID)
{
	list<int>::iterator i = find(m_BanCIDList.begin(), m_BanCIDList.end(), nCID);
	if (i!=m_BanCIDList.end())
		return true;
	else
		return false;
}

void MMatchStage::AddObject(const MUID& uid, MMatchObject* pObj)
{
	m_ObjUIDCaches.Insert(uid, pObj);


	MMatchObject* pObject = pObj;
	if (IsEnabledObject(pObject))
	{
		if (IsAdminGrade(pObject->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount++;
		}
	}

	if (GetObjCount() == 1)
	{
		SetMasterUID(uid);
	}

	m_VoteMgr.AddVoter(uid);
}

MMatchObjectMap::iterator MMatchStage::RemoveObject(const MUID& uid)
{
	m_VoteMgr.RemoveVoter(uid);
	if( CheckUserWasVoted(uid) )
	{
		m_VoteMgr.StopVote( uid );
	}

	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uid);
	if (pObj) {
		// 어드민 유저 관리
		if (IsAdminGrade(pObj->GetAccountInfo()->m_nUGrade))
		{
			m_nAdminObjectCount--;
			if (m_nAdminObjectCount < 0) m_nAdminObjectCount = 0;
		}

		LeaveBattle(pObj);
		pObj->SetStageUID(MUID(0,0));
		pObj->SetForcedEntry(false);
		pObj->SetPlace(MMP_LOBBY);
		pObj->SetStageListTransfer(true);
	}

	MMatchObjectMap::iterator i = m_ObjUIDCaches.find(uid);
	if (i==m_ObjUIDCaches.end()) 
	{
		return i;
	}

	MMatchObjectMap::iterator itorNext = m_ObjUIDCaches.erase(i);

	if (m_ObjUIDCaches.empty())
		ChangeState(STAGE_STATE_CLOSE);
	else
	{
		if (uid == GetMasterUID())
		{
			if ((GetState() == STAGE_STATE_RUN) && (GetObjInBattleCount()>0))
				SetMasterUID(RecommandMaster(true));
			else
				SetMasterUID(RecommandMaster(false));
		}
	}

	return itorNext;
}

bool MMatchStage::KickBanPlayer(const char* pszName, bool bBanPlayer)
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = i->second;
		if (pObj->GetCharInfo() == NULL) 
			continue;
		if (_stricmp(pObj->GetCharInfo()->m_szName, pszName) == 0) {
			if (bBanPlayer)
				AddBanList(pObj->GetCharInfo()->m_nCID);	// Ban

			pServer->StageLeaveBattle(pObj->GetUID(), GetUID());
			pServer->StageLeave(pObj->GetUID(), GetUID());
			return true;
		}
	}
	return false;
}

const MUID MMatchStage::RecommandMaster(bool bInBattleOnly)
{
	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)(*i).second;
		if (bInBattleOnly && (pObj->GetEnterBattle() == false))
			continue;
		return (*i).first;
	}
	return MUID(0,0);
}

void MMatchStage::EnterBattle(MMatchObject* pObj)
{
	pObj->OnEnterBattle();

	if (GetState() == STAGE_STATE_RUN)
	{
		if (pObj->IsForcedEntried())
		{
			if (m_StageSetting.IsWaitforRoundEnd())
			{
				pObj->SetAlive(false);
			}

			m_WorldItemManager.RouteAllItems(pObj);

			MMatchServer::GetInstance()->ResponseRoundState(pObj, GetUID());
		}

		if (m_pRule)
		{
			MUID uidChar = pObj->GetUID();
			m_pRule->OnEnterBattle(uidChar);
		}
	}

	pObj->SetForcedEntry(false);

}

void MMatchStage::LeaveBattle(MMatchObject* pObj)
{
	if ((GetState() == STAGE_STATE_RUN) && (m_pRule))
	{
		MUID uidChar = pObj->GetUID();
		m_pRule->OnLeaveBattle(uidChar);
	}

	pObj->OnLeaveBattle();

	// Remove the object's bots.
	if (!(pObj->GetPlayerFlags() & MTD_PlayerFlags_Bot))
	{
		auto it = std::find_if(std::begin(Bots), std::end(Bots), [&](auto&& Bot) {
			return Bot.OwnerUID == pObj->GetUID();
		});
		if (it != std::end(Bots))
		{
			MGetMatchServer()->StageLeaveBattle(pObj->GetUID(), GetUID(), it->BotUID);
			Bots.erase(it);
		}
	}
}

bool MMatchStage::CheckTick(u64 nClock)
{
	if (nClock - m_nLastTick < MTICK_STAGE) return false;
	return true;
}

void MMatchStage::UpdateWorldItems()
{
}

void MMatchStage::ResetTeams()
{
	for (auto&& Object : GetObjectList())
	{
		if (Object->GetEnterBattle())
			continue;

		Object->SetTeam(GetRecommandedTeam());
		MGetMatchServer()->StageTeam(Object->GetUID(), GetUID(), Object->GetTeam());
	}
}

void MMatchStage::Tick(u64 nClock)
{
	switch (GetState())
	{
	case STAGE_STATE_STANDBY:
		{

		}
		break;
	case STAGE_STATE_COUNTDOWN:
		{
			OnStartGame();
			ChangeState(STAGE_STATE_RUN);
		}
		break;
	case STAGE_STATE_RUN:
		{
			if (m_pRule) 
			{
				m_WorldItemManager.Update();

				if (m_pRule->Run() == false) 
				{
					OnFinishGame();

					if (GetStageType() == MST_NORMAL)
						ChangeState(STAGE_STATE_STANDBY);
					else
						ChangeState(STAGE_STATE_CLOSE);
				}
			}
		}
		break;
	}

	if (nClock - LastPhysicsTick >= 10)
	{
		MovingWeaponMgr.Update((nClock - LastPhysicsTick) / 1000.0f);
		LastPhysicsTick = nClock;
		UpdateWorldItems();
	}

	m_VoteMgr.Tick(nClock);

	if (IsChecksumUpdateTime(nClock))
		UpdateChecksum(nClock);

	m_nLastTick = nClock;

	if ((m_ObjUIDCaches.empty()) && (GetState() != STAGE_STATE_CLOSE))
	{
		ChangeState(STAGE_STATE_CLOSE);
	}

}

MMatchRule* MMatchStage::CreateRule(MMATCH_GAMETYPE nGameType)
{
	switch (nGameType)
	{
	case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
		{
			return (new MMatchRuleSoloDeath(this));
		}
		break;
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		{
			return (new MMatchRuleTeamDeath(this));
		}
		break;
	case MMATCH_GAMETYPE_GLADIATOR_SOLO:
		{
			return (new MMatchRuleSoloGladiator(this));
		}
		break;
	case MMATCH_GAMETYPE_GLADIATOR_TEAM:
		{
			return (new MMatchRuleTeamGladiator(this));
		}
		break;
	case MMATCH_GAMETYPE_ASSASSINATE:
		{
			return (new MMatchRuleAssassinate(this));
		}
		break;
	case MMATCH_GAMETYPE_TRAINING:
		{
			return (new MMatchRuleTraining(this));
		}
		break;
#ifdef _QUEST
	case MMATCH_GAMETYPE_SURVIVAL:
		{
			return (new MMatchRuleSurvival(this));
		}
		break;
	case MMATCH_GAMETYPE_QUEST:
		{
			return (new MMatchRuleQuest(this));
		}
		break;
#endif
	case MMATCH_GAMETYPE_BERSERKER:
		{
			return (new MMatchRuleBerserker(this));
		}
		break;
	case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
		{
			return (new MMatchRuleTeamDeath2(this));
		}
		break;
	case MMATCH_GAMETYPE_DUEL:
		{
			return (new MMatchRuleDuel(this));
		}
		break;
	case MMATCH_GAMETYPE_SKILLMAP:
	{
								 return (new MMatchRuleSkillmap(this));
	}
		break;
	case MMATCH_GAMETYPE_GUNGAME:
		{
			return (new MMatchRuleGunGame(this));
		}
		break;
	default:
		{
			_ASSERT(0);
		}
	}
	return NULL;
}

void MMatchStage::ChangeRule(MMATCH_GAMETYPE nRule)
{
	if ((m_pRule) && (m_pRule->GetGameType() == nRule)) return;

	if ((nRule < 0) || (nRule >= MMATCH_GAMETYPE_MAX))
	{
		MMatchServer::GetInstance()->LOG(MMatchServer::LOG_DEBUG, "ChangeRule Failed(%d)", nRule);
		return;
	}

	if (m_pRule) 
	{
		delete m_pRule;
		m_pRule = NULL;
	}

	m_pRule = CreateRule(nRule);
}

MMatchTeam MMatchStage::GetRecommandedTeam()
{
	auto GameType = GetStageSetting()->GetGameType();
	auto&& GTMgr = *MGetGameTypeMgr();

	if (GTMgr.IsTeamGame(GameType))
	{
		int nRed, nBlue;
		GetTeamMemberCount(&nRed, &nBlue, NULL, false);

		if (nRed <= nBlue)
			return MMT_RED;
		else
			return MMT_BLUE;
	}
	
	if (GTMgr.IsQuestDerived(GameType))
	{
		// ZActor::GetTeamID returns MMT_BLUE, so we return MMT_RED so that the players are on a
		// different team and friendly/enemy fire works correctly.
		return MMT_RED;
	}

	return MMT_ALL;
}

void MMatchStage::PlayerTeam(const MUID& uidPlayer, MMatchTeam nTeam)
{
	auto i = m_ObjUIDCaches.find(uidPlayer);
	if (i==m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = i->second;
	pObj->SetTeam(nTeam);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, nTeam, pObj->GetStageState());

	GetRule()->OnTeam(uidPlayer, nTeam);
}

void MMatchStage::PlayerState(const MUID& uidPlayer, MMatchObjectStageState nStageState)
{
	auto i = m_ObjUIDCaches.find(uidPlayer);
	if (i==m_ObjUIDCaches.end())
		return;

	MMatchObject* pObj = (MMatchObject*)(*i).second;

	pObj->SetStageState(nStageState);

	MMatchStageSetting* pSetting = GetStageSetting();
	pSetting->UpdateCharSetting(uidPlayer, pObj->GetTeam(), pObj->GetStageState());
}

template<size_t size> bool _GetUserGradeIDName(MMatchUserGradeID gid, char(&sp_name)[size]) {
	return _GetUserGradeIDName(gid, sp_name, size);
}

bool _GetUserGradeIDName(MMatchUserGradeID gid, char* sp_name, int maxlen)
{
	if(gid == MMUG_DEVELOPER) 
	{ 
		if(sp_name) {
			strcpy_safe(sp_name, maxlen, "개발자");
		}
		return true; 
	}
	else if(gid == MMUG_ADMIN) {
		
		if(sp_name) { 
			strcpy_safe(sp_name, maxlen, "운영자");
		}
		return true; 
	}

	return false;
}

bool MMatchStage::StartGame()
{
	int nPlayer = GetCountableObjCount();
	if (nPlayer > m_StageSetting.GetMaxPlayers())
	{
		char szMsg[ 256];
		sprintf_safe(szMsg, (char*)MErrStr( MERR_PERSONNEL_TOO_MUCH));

		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr(szMsg));
		MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		return false;
	}

	bool bResult = true;
	bool bNotReadyExist = false;

	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {

		MMatchObject* pObj = i->second;

		if ((GetMasterUID() != (*i).first) && (pObj->GetStageState() != MOSS_READY)) {
			if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
				continue;

			bNotReadyExist = true;
			bResult = false;

			char szMsg[128];
			char sp_name[256];

			if(_GetUserGradeIDName(pObj->GetAccountInfo()->m_nUGrade,sp_name))
				sprintf_safe(szMsg, (char*)MErrStr( MERR_HE_IS_NOT_READY), sp_name);
			else
				sprintf_safe(szMsg, (char*)MErrStr( MERR_HE_IS_NOT_READY), pObj->GetName());

			MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr(szMsg));
			MMatchServer::GetInstance()->RouteToStage(GetUID(), pCmd);

		}
	}

	if (bNotReadyExist)
	{
		MCommand* pCmdNotReady = MMatchServer::GetInstance()->CreateCommand( MC_GAME_START_FAIL, MUID(0, 0) );
		if( 0 == pCmdNotReady )
		{
			mlog( "MMatchStage::StartGame - 커맨드 생성 실패.\n" );
			bResult = false;
		}
		pCmdNotReady->AddParameter( new MCmdParamInt(ALL_PLAYER_NOT_READY) );
		pCmdNotReady->AddParameter( new MCmdParamInt(0) );

		MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( GetMasterUID() );
		if( IsEnabledObject(pMaster) )
		{
			MMatchServer::GetInstance()->RouteToListener( pMaster, pCmdNotReady );
		}
		else
		{
			delete pCmdNotReady;
			bResult = false;
		}
	}

	if (!QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(GetStageSetting()->GetGameType())) return false;
	}

#ifdef _QUEST_ITEM
	if (QuestTestServer())
	{
		MSTAGE_SETTING_NODE* pNode = GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog( "MMatchServer::CharFinalize - 스테이지 셋팅 노드 찾기 실패.\n" );
			return false;
		}

		if( MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) )
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog( "MMatchStage::StartGame - Quest rule로 포인터 형변환 실패.\n" );
				return false;
			}

			if( !pRuleQuest->PrepareStart() )
			{
				MCommand* pCmdNotReady = MMatchServer::GetInstance()->CreateCommand( MC_GAME_START_FAIL, MUID(0, 0) );
				pCmdNotReady->AddParameter( new MCmdParamInt(QUEST_START_FAILED_BY_SACRIFICE_SLOT) );
				pCmdNotReady->AddParameter( new MCmdParamInt(0) );

				MMatchObject* pMaster = MMatchServer::GetInstance()->GetObject( GetMasterUID() );
				if( IsEnabledObject(pMaster) )
				{
					MMatchServer::GetInstance()->RouteToListener( pMaster, pCmdNotReady );
				}
				else
				{
					delete pCmdNotReady;
					bResult = false;
				}


				#ifdef _DEBUG
					mlog( "MMatchServer::OnStageStart - 슬롯 조건 검사에서 실패하여 게임을 시작할수 없음.\n" );
				#endif

				return false;
			}
		}
	}
#endif

	MMatchObject* pMasterObj = MMatchServer::GetInstance()->GetObject(GetMasterUID());
	if (pMasterObj && IsAdminGrade(pMasterObj) && pMasterObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		bResult = true;
	
	if (bResult == true)
	{
		for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
		{
			MMatchObject* pObj = i->second;
			pObj->SetLaunchedGame(true);
		}

		ChangeState(STAGE_STATE_COUNTDOWN);
	}

	BspObject = MGetMatchServer()->LagComp.GetBspObject(m_StageSetting.GetMapName());

	if (!MGetServerConfig()->HasGameData() && GetStageSetting()->GetNetcode() == NetcodeType::ServerBased)
	{
		GetStageSetting()->SetNetcode(NetcodeType::P2PAntilead);
		MGetMatchServer()->RouteToStage(GetUID(), MGetMatchServer()->CreateCmdResponseStageSetting(GetUID()));
	}

	return bResult;
}

void MMatchStage::SetStageType(MMatchStageType nStageType)
{
	if (m_nStageType == nStageType) return;

	switch (nStageType)
	{
	case MST_NORMAL:
		{
			m_StageSetting.SetTeamWinThePoint(false);
		}
		break;
	case MST_LADDER:
		{
			m_StageSetting.SetTeamWinThePoint(true);
		}
		break;
	}

	m_nStageType = nStageType;
}

void MMatchStage::OnStartGame()
{
	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = i->second;
		pObj->SetAllRoundDeathCount(0);	
		pObj->SetAllRoundKillCount(0);
		pObj->SetVoteState( false );
	}
	
	for (int i = 0; i < MMT_END; i++)
	{
		m_Teams[i].nScore = 0;
		m_Teams[i].nSeriesOfVictories = 0;
	}

	if (m_pRule)
	{
		m_pRule->Begin();
	}

	m_nStartTime = MMatchServer::GetInstance()->GetTickTime();

	m_WorldItemManager.OnStageBegin(&m_StageSetting);

	if (GetStageType() == MST_NORMAL)
		MMatchServer::GetInstance()->StageLaunch(GetUID());
}

void MMatchStage::OnFinishGame()
{
	m_WorldItemManager.OnStageEnd();

	if (m_pRule)
	{
		m_pRule->End();
	}
	MMatchServer::GetInstance()->StageFinishGame(GetUID());

	if ((MGetServerConfig()->GetServerMode() == MSM_LADDER) || (MGetServerConfig()->GetServerMode() == MSM_CLAN) || 
		(MGetServerConfig()->GetServerMode() == MSM_TEST))
	{
		if ((m_nStageType == MST_LADDER) && (GetStageSetting()->IsTeamPlay()))
		{
			MMatchTeam nWinnerTeam = MMT_RED;
			bool bIsDrawGame = false;
			int nRedTeamCount=0, nBlueTeamCount=0;

			GetTeamMemberCount(&nRedTeamCount, &nBlueTeamCount, NULL, true);

			if ((m_Teams[MMT_RED].nScore > m_Teams[MMT_BLUE].nScore) || (nBlueTeamCount==0))
			{
				nWinnerTeam = MMT_RED;
			}
			else if ((m_Teams[MMT_RED].nScore < m_Teams[MMT_BLUE].nScore) || (nRedTeamCount==0))
			{
				nWinnerTeam = MMT_BLUE;
			}
			else
			{
				bIsDrawGame = true;
			}

			MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
			if (pTeamGameStrategy)
			{
				pTeamGameStrategy->SavePointOnFinishGame(this, nWinnerTeam, bIsDrawGame, &m_Teams[MMT_RED].LadderInfo,
					&m_Teams[MMT_BLUE].LadderInfo);
			};
		}
	}

	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = i->second;
		if (pObj->GetStageState())
			pObj->SetStageState(MOSS_NONREADY);
	}

	m_nStartTime = 0;
}

bool MMatchStage::CheckBattleEntry()
{
	bool bResult = true;
	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++) {
		MMatchObject* pObj = i->second;
		if (pObj->IsLaunchedGame())
		{
			if (pObj->GetEnterBattle() == false) bResult = false;
		}
	}
	return bResult;
}

void MMatchStage::RoundStateFromClient(const MUID& uidStage, int nState, int nRound)
{
	
}

int MMatchStage::GetObjInBattleCount()
{
	int nCount = 0;
	for (auto itor=GetObjBegin(); itor!=GetObjEnd(); ++itor)
	{
		MMatchObject* pObj = itor->second;
		if (pObj->GetEnterBattle() == true)
		{
			nCount++;
		}
	}

	return nCount;
}

void MMatchStage::SetOwnerChannel(const MUID& uidOwnerChannel, int nIndex)
{
	m_uidOwnerChannel = uidOwnerChannel;
	m_nIndex = nIndex;
}

void MMatchStage::ObtainWorldItem(MMatchObject* pObj, const int nItemUID)
{
	if (GetState() != STAGE_STATE_RUN) return;
	m_WorldItemManager.Obtain(pObj, short(nItemUID));
}

void MMatchStage::RequestSpawnWorldItem(MMatchObject* pObj, const int nItemID,
	const float x, const float y, const float z)
{
	if (GetState() != STAGE_STATE_RUN) return;

	if (nItemID < 100) return;

	m_WorldItemManager.SpawnDynamicItem(nItemID, x, y, z);
}

void MMatchStage::SpawnServerSideWorldItem(MMatchObject* pObj, const int nItemID, 
							const float x, const float y, const float z, 
							int nLifeTime, int* pnExtraValues )
{
	if (GetState() != STAGE_STATE_RUN) return;

	m_WorldItemManager.SpawnDynamicItem(nItemID, x, y, z, nLifeTime, pnExtraValues );
}

bool MMatchStage::IsApplyTeamBonus()
{
 	if ((m_StageSetting.IsTeamPlay()) && (m_TeamBonus.bApplyTeamBonus == true))
	{
		return true;
	}
	return false;
}

void MMatchStage::OnInitRound()
{
	m_TeamBonus.bApplyTeamBonus = false;

	for (int i = 0; i < MMT_END; i++)
	{
		m_Teams[i].nTeamBonusExp = 0;
		m_Teams[i].nTeamTotalLevel = 0;
		m_Teams[i].nTotalKills = 0;
	}

	int nRedTeamCount = 0, nBlueTeamCount = 0;

	for (auto i=GetObjBegin(); i!=GetObjEnd(); i++) {
		MMatchObject* pObj = i->second;
		if (pObj->GetEnterBattle() == true)
		{
			pObj->OnInitRound();

			if (m_StageSetting.IsTeamPlay())
			{
				if (pObj->GetTeam() == MMT_RED) 
				{
					nRedTeamCount++;
					if (pObj->GetCharInfo())
						m_Teams[MMT_RED].nTeamTotalLevel += pObj->GetCharInfo()->m_nLevel;
				}
				else if (pObj->GetTeam() == MMT_BLUE) 
				{
					nBlueTeamCount++;
					if (pObj->GetCharInfo())
						m_Teams[MMT_BLUE].nTeamTotalLevel += pObj->GetCharInfo()->m_nLevel;
				}
			}
		}
	}


	if (m_StageSetting.IsTeamPlay())
	{
		if ((nRedTeamCount >= NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS) && 
		    (nBlueTeamCount >= NUM_APPLYED_TEAMBONUS_TEAM_PLAYERS))
		{
			m_TeamBonus.bApplyTeamBonus = true;
		}
	}
}

void MMatchStage::AddTeamBonus(int nExp, MMatchTeam nTeam)
{
	m_Teams[nTeam].nTeamBonusExp += nExp;
}

void MMatchStage::OnApplyTeamBonus(MMatchTeam nTeam)
{
	if (GetStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
	{
		for (auto i=GetObjBegin(); i!=GetObjEnd(); i++)
		{
			MMatchObject* pObj = i->second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					int nAddedExp = 0, nChrLevel = 0;
					if (pObj->GetCharInfo()) nChrLevel = pObj->GetCharInfo()->m_nLevel;
					if (m_Teams[nTeam].nTeamTotalLevel != 0)
					{
						nAddedExp = (int)(m_Teams[nTeam].nTeamBonusExp * ((float)nChrLevel / (float)m_Teams[nTeam].nTeamTotalLevel));
					}
					MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nAddedExp);
				}
			}
		}
	}
	else
	{
		int TotalKills = 0;
		for (auto i=GetObjBegin(); i!=GetObjEnd(); i++)
		{
			MMatchObject* pObj = i->second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					TotalKills += pObj->GetKillCount() + 1;
				}
			}
		}

		if (TotalKills == 0)
			TotalKills = 10000000;


		for (auto i=GetObjBegin(); i!=GetObjEnd(); i++)
		{
			MMatchObject* pObj = i->second;
			if (pObj->GetEnterBattle() == true)
			{
				if ((pObj->GetTeam() == nTeam) && (pObj->GetGameInfo()->bJoinedGame == true))
				{
					int nAddedExp = 0;
					nAddedExp = (int)(m_Teams[nTeam].nTeamBonusExp * ((float)(pObj->GetKillCount() + 1) / (float)TotalKills));
					int nMaxExp = (pObj->GetCharInfo()->m_nLevel * 30 - 10) * 2 * pObj->GetKillCount();
					if (nAddedExp > nMaxExp) nAddedExp = nMaxExp;
					MMatchServer::GetInstance()->ApplyObjectTeamBonus(pObj, nAddedExp);
				}
			}
		}
	}
}

void MMatchStage::OnRoundEnd_FromTeamGame(MMatchTeam nWinnerTeam)
{
	if (IsApplyTeamBonus())
	{
		OnApplyTeamBonus(nWinnerTeam);
	}
	m_Teams[nWinnerTeam].nScore++;

	m_Teams[nWinnerTeam].nSeriesOfVictories++;
	m_Teams[NegativeTeam(nWinnerTeam)].nSeriesOfVictories = 0;

	if (CheckAutoTeamBalancing())
	{
		ShuffleTeamMembers();
	}
}

void MMatchStage::SetLadderTeam(MMatchLadderTeamInfo* pRedLadderTeamInfo, MMatchLadderTeamInfo* pBlueLadderTeamInfo)
{
	memcpy(&m_Teams[MMT_RED].LadderInfo, pRedLadderTeamInfo, sizeof(MMatchLadderTeamInfo));
	memcpy(&m_Teams[MMT_BLUE].LadderInfo, pBlueLadderTeamInfo, sizeof(MMatchLadderTeamInfo));
}

void MMatchStage::OnCommand(MCommand* pCommand)
{
	if (m_pRule) m_pRule->OnCommand(pCommand);
}

int MMatchStage::GetMinPlayerLevel()
{
	int nMinLevel = MAX_CHAR_LEVEL;

	for (auto i=GetObjBegin(); i!=GetObjEnd(); i++)
	{
		MMatchObject* pObj = i->second;
		if (!IsEnabledObject(pObj)) continue;

		if (nMinLevel > pObj->GetCharInfo()->m_nLevel) nMinLevel = pObj->GetCharInfo()->m_nLevel;
	}

	return nMinLevel;
}

bool MMatchStage::CheckUserWasVoted( const MUID& uidPlayer )
{
	MMatchObject* pPlayer = MMatchServer::GetInstance()->GetObject( uidPlayer );
	if( !IsEnabledObject(pPlayer) )
		return false;

	MVoteMgr* pVoteMgr = GetVoteMgr();
	if( 0 == pVoteMgr )
		return false;

	if( !pVoteMgr->IsGoingOnVote() )
		return false;

	MVoteDiscuss* pVoteDiscuss = pVoteMgr->GetDiscuss();
	if(  0 == pVoteDiscuss )
		return false;

	string strVoteTarget = pVoteDiscuss->GetImplTarget();
	if( (0 != (strVoteTarget.size() - strlen(pPlayer->GetName()))) )
		return false;
	
	if( 0 != strncmp(strVoteTarget.c_str(),pPlayer->GetName(), strVoteTarget.size()) )
		return false;

	return true;
}

MMatchItemBonusType GetStageBonusType(MMatchStageSetting* pStageSetting)
{
	if (pStageSetting->IsQuestDrived()) return MIBT_QUEST;
	else if (pStageSetting->IsTeamPlay()) return MIBT_TEAM;

	return MIBT_SOLO;
}

void MMatchStage::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	if (m_pRule)
	{
		m_pRule->OnGameKill(uidAttacker, uidVictim);
	}
}

bool moreTeamMemberKills(MMatchObject* pObject1, MMatchObject* pObject2)
{
	return (pObject1->GetAllRoundKillCount() > pObject2->GetAllRoundKillCount());
}

void MMatchStage::ShuffleTeamMembers()
{
	if ((m_nStageType == MST_LADDER) || (m_StageSetting.IsTeamPlay() == false)) return;
	if (m_ObjUIDCaches.empty()) return;

	int nTeamMemberCount[MMT_END] = {0, };
	MMatchTeam nWinnerTeam;

	GetTeamMemberCount(&nTeamMemberCount[MMT_RED], &nTeamMemberCount[MMT_BLUE], NULL, true);
	if (nTeamMemberCount[MMT_RED] >= nTeamMemberCount[MMT_BLUE]) nWinnerTeam = MMT_RED; 
	else nWinnerTeam = MMT_BLUE;

	int nShuffledMemberCount = abs(nTeamMemberCount[MMT_RED] - nTeamMemberCount[MMT_BLUE]) / 2;
	if (nShuffledMemberCount <= 0) return;

	vector<MMatchObject*> sortedObjectList;

	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = i->second;

		if ((pObj->GetEnterBattle() == true) && (pObj->GetGameInfo()->bJoinedGame == true))
		{
			if ((pObj->GetTeam() == nWinnerTeam) && (!IsAdminGrade(pObj)))
			{
				sortedObjectList.push_back(pObj);
			}
		}
	}

	std::sort(sortedObjectList.begin(), sortedObjectList.end(), moreTeamMemberKills);

	int nCounter = 0;
	for (vector<MMatchObject*>::iterator itor = sortedObjectList.begin(); itor != sortedObjectList.end(); ++itor)
	{
		MMatchObject* pObj = (*itor);
		PlayerTeam(pObj->GetUID(), NegativeTeam(MMatchTeam(pObj->GetTeam())));
		nCounter++;

		if (nCounter >= nShuffledMemberCount) break;
	}

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_RESET_TEAM_MEMBERS, MUID(0,0));
	int nMemberCount = (int)m_ObjUIDCaches.size();
	void* pTeamMemberDataArray = MMakeBlobArray(sizeof(MTD_ResetTeamMembersData), nMemberCount);

	nCounter = 0;
	for (auto i=m_ObjUIDCaches.begin(); i!=m_ObjUIDCaches.end(); i++)
	{
		MMatchObject* pObj = i->second;
		MTD_ResetTeamMembersData* pNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pTeamMemberDataArray, nCounter);
		pNode->m_uidPlayer = pObj->GetUID();
		pNode->nTeam = (char)pObj->GetTeam();

		nCounter++;
	}

	pCmd->AddParameter(new MCommandParameterBlob(pTeamMemberDataArray, MGetBlobArraySize(pTeamMemberDataArray)));
	MEraseBlobArray(pTeamMemberDataArray);
	MMatchServer::GetInstance()->RouteToBattle(GetUID(), pCmd);
}

bool MMatchStage::CheckAutoTeamBalancing()
{
	if ((m_nStageType == MST_LADDER) || (m_StageSetting.IsTeamPlay() == false)) return false;
	if (m_StageSetting.GetAutoTeamBalancing() == false) return false;

	int nMemberCount[MMT_END] = {0, };
	GetTeamMemberCount(&nMemberCount[MMT_RED], &nMemberCount[MMT_BLUE], NULL, true);

	const int MEMBER_COUNT = 2;
	const int SERIES_OF_VICTORIES = 3;

	if ( ((nMemberCount[MMT_RED] - nMemberCount[MMT_BLUE]) >= MEMBER_COUNT) && 
		 (m_Teams[MMT_RED].nSeriesOfVictories >= SERIES_OF_VICTORIES) )
	{
		return true;
	}
	else if ( ((nMemberCount[MMT_BLUE] - nMemberCount[MMT_RED]) >= MEMBER_COUNT) && 
		 (m_Teams[MMT_BLUE].nSeriesOfVictories >= SERIES_OF_VICTORIES) )
	{
		return true;
	}

	return false;
}

void MMatchStage::GetTeamMemberCount(int* poutnRedTeamMember, int* poutnBlueTeamMember, int* poutSpecMember, bool bInBattle)
{
	if (poutnRedTeamMember) *poutnRedTeamMember = 0;
	if (poutnBlueTeamMember) *poutnBlueTeamMember = 0;
	if (poutSpecMember) *poutSpecMember = 0;

	for (auto itor=GetObjBegin(); itor!=GetObjEnd(); itor++)
	{
		MMatchObject* pObj = itor->second;

		if (((bInBattle == true) && (pObj->GetEnterBattle() == true)) || (bInBattle == false))
		{
			switch (pObj->GetTeam())
			{
			case MMT_RED:		if (poutnRedTeamMember) (*poutnRedTeamMember)++; break;
			case MMT_BLUE:		if (poutnBlueTeamMember) (*poutnBlueTeamMember)++; break;
			case MMT_SPECTATOR:	if (poutSpecMember) (*poutSpecMember)++; break;
			};
		}
	}
}

int MMatchStage::GetPlayers()
{
	int nPlayers = 0;

	for (auto i = GetObjBegin();  i != GetObjEnd();  i++)
	{
		MMatchObject* pObj = i->second;
		
		if ( IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
			continue;

		nPlayers++;
	}

	return nPlayers;
}
