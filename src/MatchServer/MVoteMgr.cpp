#include "stdafx.h"
#include "MVoteMgr.h"
#include <algorithm>
#include "MMatchServer.h"
#include "MSharedCommandTable.h"



/////////////////////////////////////////////////
// MVoteDiscuss
MVoteDiscuss::MVoteDiscuss(const MUID& uidStage)
{
	m_uidStage = uidStage;
	m_nBeginTime = MMatchServer::GetInstance()->GetGlobalClockCount();
}

MVoteDiscuss::~MVoteDiscuss()
{
	while(m_YesVoterList.size()) {
		m_YesVoterList.pop_front();
	}
	while(m_NoVoterList.size()) {
		m_NoVoterList.pop_front();
	}
}

bool MVoteDiscuss::CheckVoter(const MUID& uid)
{
	list<MUID>::iterator iYes = find(m_YesVoterList.begin(), m_YesVoterList.end(), uid);
	if (iYes!=m_YesVoterList.end())
		return true;

	list<MUID>::iterator iNo = find(m_NoVoterList.begin(), m_NoVoterList.end(), uid);
	if (iNo!=m_NoVoterList.end())
		return true;

	return false;
}

void MVoteDiscuss::Vote(const MUID& uid, MVOTE nVote)
{
	if (CheckVoter(uid))
		return;		// already voted

	if (nVote == MVOTE_YES) {
		m_YesVoterList.push_back(uid);
	} else if (nVote == MVOTE_NO) {
		m_NoVoterList.push_back(uid);
	} else {
		_ASSERT("NEVER HAPPEND");
		return;
	}
}

/////////////////////////////////////////////////
// MVoteMgr
MVoteMgr::MVoteMgr()
{
	m_pDiscuss = NULL;
}

MVoteMgr::~MVoteMgr()
{
	if (GetDiscuss()) {
		delete GetDiscuss();
		m_pDiscuss = NULL;
	}

	m_VoterMap.clear();
}

bool MVoteMgr::CheckDiscuss()
{
	MVoteDiscuss* pDiscuss = GetDiscuss();
	if (pDiscuss == NULL)
		return false;

	int nYesCount = (int)pDiscuss->GetYesVoterCount();
	int nNoCount = (int)pDiscuss->GetNoVoterCount();

//	char szLog[128]="";
//	sprintf_safe(szLog, "VOTERESULT: Y(%f), N(%f)", (float)nYesCount, (float)m_VoterMap.size() * 0.5f);
//	MMatchServer::GetInstance()->LOG(MMatchServer::LOG_PROG, szLog);

	if ( (float)nYesCount > (float)m_VoterMap.size() * 0.66f )	// 2/3이상 찬성하면.
		return true;
	else
		return false;
}

void MVoteMgr::FinishDiscuss(bool bJudge)
{
	if (GetDiscuss()) {
		MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_NOTIFY_VOTERESULT, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(GetDiscuss()->GetDiscussName()));
		pCmd->AddParameter(new MCmdParamInt(bJudge?1:0));
		MMatchServer::GetInstance()->RouteToStage(GetDiscuss()->GetStageUID(), pCmd);

		GetDiscuss()->OnJudge(bJudge);

		delete GetDiscuss();
		m_pDiscuss = NULL;
	}
	SetLastError(VOTEMGR_ERROR_OK);
}

MVoter* MVoteMgr::FindVoter(const MUID& uid)
{
	MVoterMap::iterator i = m_VoterMap.find(uid);
	if (i != m_VoterMap.end())
		return (*i).second;
	else
		return NULL;
}

void MVoteMgr::AddVoter(const MUID& uid)
{
	if (FindVoter(uid) != NULL) {
		SetLastError(VOTEMGR_ERROR_OK);
		return;
	}

	MVoter* pVoter = new MVoter(uid);
	m_VoterMap.insert(MVoterMap::value_type(uid, pVoter));

	SetLastError(VOTEMGR_ERROR_OK);
}

void MVoteMgr::RemoveVoter(const MUID& uid)
{
	MVoterMap::iterator i = m_VoterMap.find(uid);
	if (i != m_VoterMap.end()) {
		MVoter* pVoter = (*i).second;
		delete pVoter;
		m_VoterMap.erase(i);
	}

	SetLastError(VOTEMGR_ERROR_OK);
}

bool MVoteMgr::CallVote(MVoteDiscuss* pDiscuss)
{
	if (GetDiscuss()) {
		SetLastError(VOTEMGR_ERROR_VOTE_INPROGRESS);
		return false;
	}

	m_pDiscuss = pDiscuss;

	SetLastError(VOTEMGR_ERROR_OK);
	return true;
}

bool MVoteMgr::Vote(const MUID& uid, MVOTE nVote)
{
	if (GetDiscuss() == NULL) {
		SetLastError(VOTEMGR_ERROR_VOTE_NODISCUSS);
		return false;
	}

	MVoteDiscuss* pDiscuss = GetDiscuss();
	if (pDiscuss->CheckVoter(uid)) {
		SetLastError(VOTEMGR_ERROR_VOTE_ALREADY_VOTED);
		return false;
	}

	pDiscuss->Vote(uid, nVote);
	SetLastError(VOTEMGR_ERROR_OK);

	return true;
}

void MVoteMgr::Tick(u64 nClock)
{
	if (GetDiscuss() == NULL)
		return;

	if (CheckDiscuss() == true) {
		FinishDiscuss(true);
		return;
	} else {
		// 투표가능자가 없으면 투표 부결로 종료
		if (m_VoterMap.size() <= GetDiscuss()->GetYesVoterCount() + GetDiscuss()->GetNoVoterCount()) {
			FinishDiscuss(false);
			return;
		}
	}

	if (nClock - GetDiscuss()->GetBeginTime() > 60000) {	// 1분동안 미결정이면 종료
		FinishDiscuss(false);
		return;
	}
}


void MVoteMgr::StopVote( const MUID& uidUser )
{
	delete m_pDiscuss;
	m_pDiscuss = NULL;

	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject( uidUser );
	if( !IsEnabledObject(pObj) )
		return;

	MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand( MC_MATCH_VOTE_STOP, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	MMatchStage* pStage = MMatchServer::GetInstance()->FindStage( pObj->GetStageUID() );
	if( 0 == pStage )
		return;

	MMatchServer::GetInstance()->RouteToStage( pStage->GetUID(), pCmd );
}