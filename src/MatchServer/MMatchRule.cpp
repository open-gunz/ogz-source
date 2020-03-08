#include "stdafx.h"
#include "MMatchServer.h"
#include "MMatchRule.h"
#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MSharedCommandTable.h"
#include "MBlobArray.h"
#include "MMatchConfig.h"
#include "MMatchEventFactory.h"
#include "MErrorTable.h"

void MMatchRule::DebugTest()
{
#ifdef _DEBUG
	SetRoundState(MMATCH_ROUNDSTATE_FINISH);
#endif
}

//// RULE //////////////////////////////////////////////////////////////////////
MMatchRule::MMatchRule(MMatchStage* pStage)	: m_pStage(pStage), m_nRoundCount(0), m_nRoundArg(0),
				m_pGameTypeInfo(NULL), m_nRoundState(MMATCH_ROUNDSTATE_FREE)
{ 
	SetRoundStateTimer(MMatchServer::GetInstance()->GetGlobalClockCount());
}

void MMatchRule::SetRoundState(MMATCH_ROUNDSTATE nState)
{ 
	if (m_nRoundState == nState) return;

	if (nState == MMATCH_ROUNDSTATE_FINISH)	OnRoundEnd();
	else if (nState == MMATCH_ROUNDSTATE_PLAY) OnRoundBegin();

	m_nRoundState = nState;
	SetRoundStateTimer(MMatchServer::GetInstance()->GetGlobalClockCount());

	if (nState == MMATCH_ROUNDSTATE_COUNTDOWN)
	{
		InitRound();
	}

	MMatchServer::GetInstance()->ResponseRoundState(GetStage()->GetUID());

}

void MMatchRule::InitRound()
{
	if (m_pStage == NULL) 
	{
		_ASSERT(0);
		return;
	}

	// 팀전 보너스 초기화
	m_pStage->OnInitRound();

	// 월드아이템 초기화
	m_pStage->m_WorldItemManager.OnRoundBegin();

	// 시간제한 잔량 공지 초기화
	SetLastTimeLimitAnnounce(INT_MAX);
}

void MMatchRule::OnBegin()
{
}
void MMatchRule::OnEnd()
{
}
void MMatchRule::OnRoundBegin()
{
}
void MMatchRule::OnRoundEnd()
{
}

void MMatchRule::OnRoundTimeOut()
{
}

bool MMatchRule::OnCheckBattleTimeOut(unsigned int tmTimeSpend)
{
	int nLimitTime = GetStage()->GetStageSetting()->GetLimitTime() * 60 * 1000;
	if (nLimitTime <= STAGESETTING_LIMITTIME_UNLIMITED) return false;

//	int nLimitTime = 1 * 60 * 1000;
	int nTimeRemain = nLimitTime - tmTimeSpend;

	if ((unsigned int)nLimitTime < tmTimeSpend)
		return true;
	if ((nLimitTime == 0) && (tmTimeSpend > 24*60*60*1000))
		return true;

	if ((GetLastTimeLimitAnnounce() > 10) && (nTimeRemain < 10*1000))  {
		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr( (char*)MErrStr( MERR_TIME_10REMAINING)));

		MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
		SetLastTimeLimitAnnounce(10);
		return false;
	} else if ((GetLastTimeLimitAnnounce() > 30) && (nTimeRemain < 30*1000)) {
		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr( (char*)MErrStr( MERR_TIME_30REMAINING)));

		MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
		SetLastTimeLimitAnnounce(30);
		return false;
	} else if ((GetLastTimeLimitAnnounce() > 60) && (nTimeRemain < 60*1000)) {
		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUInt(0));
		pCmd->AddParameter(new MCmdParamStr( (char*)MErrStr( MERR_TIME_60REMAINING)));

		MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
		SetLastTimeLimitAnnounce(60);
		return false;
	}

	return false;
}

bool MMatchRule::OnRun()
{
	auto nClock = MMatchServer::GetInstance()->GetGlobalClockCount();

	switch (GetRoundState())
	{
	case MMATCH_ROUNDSTATE_PREPARE:
		{
			if (GetStage()->CheckBattleEntry() == true) 
			{
				if (OnCheckEnableBattleCondition())
				{
					SetRoundState(MMATCH_ROUNDSTATE_COUNTDOWN);
				}
				else
				{
					SetRoundState(MMATCH_ROUNDSTATE_FREE);
				}
				return true;
			} 
			else
			{
				// Drop Timedout Player
				if (nClock - GetRoundStateTimer() > 30*1000)
				{
					// 아직 만들지 않았다.
				} 

				
				return true;
			}
		}
		break;
	case MMATCH_ROUNDSTATE_COUNTDOWN:
		{
			if (nClock - GetRoundStateTimer() > 2*1000) {
				SetRoundState(MMATCH_ROUNDSTATE_PLAY);
				return true;
			} else {
				// Countdown proceeding now, Do Nothing..
				return true;
			}

		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
		{
			// 게임하지 못할 상황이면 Free상태로 변환
			if (!OnCheckEnableBattleCondition())
			{
				SetRoundState(MMATCH_ROUNDSTATE_FREE);
			}

			if (OnCheckRoundFinish())
			{
				// 정상으로 퀘스트 종료.
				// OnRoundTimeOut();
				SetRoundState( MMATCH_ROUNDSTATE_FINISH );
			} 
			else if (OnCheckBattleTimeOut(static_cast<u32>(nClock - GetRoundStateTimer()))) 
			{
				// Make Draw Game...

				OnRoundTimeOut();
				SetRoundState(MMATCH_ROUNDSTATE_FINISH);
			} 
			else 
			{
				// They still playing the game..
			}

			CheckOnGameEvent();
			
			return true;
		}
		break;
	case MMATCH_ROUNDSTATE_FINISH:
		{
			if (nClock - GetRoundStateTimer() > 3*1000) {	// 3초
				if (RoundCount() == true)
				{
					SetRoundState(MMATCH_ROUNDSTATE_PREPARE);
				}
				else
					SetRoundState(MMATCH_ROUNDSTATE_EXIT);

				return true;
			} else {
				// Spend time, Do nothing
				return true;
			}
		}
		break;
	case MMATCH_ROUNDSTATE_EXIT:
		{
			// End game
			return false;
		}
		break;
	case MMATCH_ROUNDSTATE_FREE:
		{
			if (OnCheckEnableBattleCondition())
			{
				// 게임 시작
				SetRoundState(MMATCH_ROUNDSTATE_PREPARE);
			}

			return true;
		}
		break;
	default:
		{
			_ASSERT(0);
		}
	}

	return false;	// false return make RoundExit
}

bool MMatchRule::Run()
{
	const bool bResult = OnRun();
	// CheckOnGameEvent();
	RunOnGameEvent();
	return bResult;
}

void MMatchRule::Begin()
{
	m_nRoundCount = 0;
	SetRoundState(MMATCH_ROUNDSTATE_PREPARE);
	SetRoundStateTimer( MMatchServer::GetInstance()->GetGlobalClockCount() );
    
	InitOnBeginEventManager();
	InitOnGameEventManager();
	InitOnEndEventManager();
	
	OnBegin();

	CheckOnBeginEvent();
	RunOnBeginEvent();
}

void MMatchRule::End()
{
	OnEnd();

	CheckOnEndEvent();
	RunOnEndEvent();
}

void MMatchRule::CalcTeamBonus(MMatchObject* pAttacker, MMatchObject* pVictim,
								int nSrcExp, int* poutAttackerExp, int* poutTeamExp)
{
	*poutAttackerExp = nSrcExp;
	*poutTeamExp = 0;
}


void MMatchRule::InitOnBeginEventManager()
{
	if( GetGameType() != GetOnBeginEventManager().GetLastSetGameType() )
	{
		EventPtrVec EvnPtrVec;
		if( !MMatchEventFactoryManager::GetInstance().GetEventList(GetGameType(), ET_BEGIN, EvnPtrVec) )
		{
			ASSERT( 0 && "이벤트 리스트 생성 실패.\n" );
			mlog( "MMatchRule::InitOnBeginEventManager - 리스트 생성 실패.\n" );
			MMatchEventManager::ClearEventPtrVec( EvnPtrVec );
			return;
		}
		GetOnBeginEventManager().ChangeEventList( EvnPtrVec );
	}
}


void MMatchRule::InitOnGameEventManager()
{
	if( GetGameType() != GetOnGameEventManager().GetLastSetGameType() )
	{
		EventPtrVec EvnPtrVec;
		if( !MMatchEventFactoryManager::GetInstance().GetEventList(GetGameType(), ET_ONGAME, EvnPtrVec) )
		{
			ASSERT( 0 && "이벤트 리스트 생성 실패.\n" );
			mlog( "MMatchRule::InitOnGameEventManager - 리스트 생성 실패.\n" );
			MMatchEventManager::ClearEventPtrVec( EvnPtrVec );
			return;
		}
		GetOnGameEventManager().ChangeEventList( EvnPtrVec );
	}
}


void MMatchRule::InitOnEndEventManager()
{
	if( GetGameType() != GetOnGameEventManager().GetLastSetGameType() )
	{
		EventPtrVec EvnPtrVec;
		if( !MMatchEventFactoryManager::GetInstance().GetEventList(GetGameType(), ET_END, EvnPtrVec) )
		{
			ASSERT( 0 && "이벤트 리스트 생성 실패.\n" );
			mlog( "MMatchRule::InitOnEndEventManager - 리스트 생성 실패.\n" );
			MMatchEventManager::ClearEventPtrVec( EvnPtrVec );
			return;
		}
		GetOnEndEventManager().ChangeEventList( EvnPtrVec );
	}
	
}


void MMatchRule::CheckOnBeginEvent()
{
	if( !GetOnBeginEventManager().Empty() )
	{
		MMatchObject* pObj;
		auto dwClock = MMatchServer::GetInstance()->GetGlobalClockCount();
		GetOnBeginEventManager().StartNewEvent();
		for (auto i=GetStage()->GetObjBegin(); i!=GetStage()->GetObjEnd(); i++)
		{
			pObj = i->second;
			if( MMP_STAGE == pObj->GetPlace() )
				GetOnBeginEventManager().CheckEventObj( pObj, dwClock );
		}

		GetOnBeginEventManager().SetLastCheckTime( dwClock );
	}
}


void MMatchRule::CheckOnGameEvent()
{
	if( MMATCH_ROUNDSTATE_PLAY == GetRoundState() )
	{
		// 게임상에 아무도 없으면 exit
		int nInGamePlayer = 0;

		MMatchObject*	pObj = nullptr;
		auto			dwClock = MMatchServer::GetInstance()->GetGlobalClockCount();

		for (auto i=GetStage()->GetObjBegin(); i!=GetStage()->GetObjEnd(); i++)
		{
			pObj = i->second;
			if( (pObj->GetEnterBattle() == true) || pObj->IsLaunchedGame() )
			{
				GetOnGameEventManager().CheckEventObj( pObj, dwClock );
				++nInGamePlayer;
			}
		}

		GetOnGameEventManager().SetLastCheckTime( dwClock );

		if (nInGamePlayer == 0) 
			SetRoundState( MMATCH_ROUNDSTATE_EXIT );
	}	
}


void MMatchRule::CheckOnEndEvent()
{
	if( !GetOnEndEventManager().Empty() )
	{
		MMatchObject*	pObj;
		auto			dwClock = MMatchServer::GetInstance()->GetGlobalClockCount();

		GetOnEndEventManager().StartNewEvent();
		for (auto i=GetStage()->GetObjBegin(); i!=GetStage()->GetObjEnd(); i++)
		{
			pObj = i->second;
			if( MMP_BATTLE == pObj->GetPlace() )
				GetOnEndEventManager().CheckEventObj( pObj, dwClock );
		}

		GetOnEndEventManager().SetLastCheckTime( dwClock );
	}
}


void MMatchRule::RunOnBeginEvent()
{
	GetOnBeginEventManager().Run();	
}


void MMatchRule::RunOnGameEvent()
{
	GetOnGameEventManager().Run();
	GetOnGameEventManager().StartNewEvent();
}


void MMatchRule::RunOnEndEvent()
{
	GetOnEndEventManager().Run();
}
//////////////////////////////////////////////////////////////////////////////////