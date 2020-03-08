#pragma once

#include "MUID.h"

#include <list>
#include <map>


enum MVOTE {
	MVOTE_GIVEUP,
	MVOTE_YES,
	MVOTE_NO
};


class MVoter {
protected:
	MUID	m_uidVoter;
public:
	MVoter(MUID uid)	{ m_uidVoter = uid; }
	MUID GetID()		{ return m_uidVoter; }
};
class MVoterMap : public map<MUID, MVoter*> {};


class MVoteDiscuss {
protected:
	MUID			m_uidStage;			// 스테이지UID
	string			m_strDiscuss;		// 안건
	u64				m_nBeginTime;		// 발의된 시간
	std::list<MUID>		m_YesVoterList;		// 찬성자
	std::list<MUID>		m_NoVoterList;		// 반대자

public:
	MVoteDiscuss(const MUID& uidStage);
	virtual ~MVoteDiscuss();

	MUID GetStageUID()	{ return m_uidStage; }
	const char* GetDiscussName()	{ return m_strDiscuss.c_str(); }
	auto GetBeginTime() const	{ return m_nBeginTime; }
	size_t GetYesVoterCount()	{ return m_YesVoterList.size(); }
	size_t GetNoVoterCount()	{ return m_NoVoterList.size(); }

	bool CheckVoter(const MUID& uid);	// 투표자인지 검사
	void Vote(const MUID& uid, MVOTE nVote);
public:
	virtual bool OnJudge(bool bJudge) = 0;
	virtual string GetImplTarget() = 0;
};


class MVoteMgr {
public:
	enum VOTEMGR_ERROR {
		VOTEMGR_ERROR_OK,
		VOTEMGR_ERROR_UNKNOWN,
		VOTEMGR_ERROR_VOTE_NODISCUSS,
		VOTEMGR_ERROR_VOTE_INPROGRESS,
		VOTEMGR_ERROR_VOTE_ALREADY_VOTED
	};

protected:
	MVoterMap			m_VoterMap;
	MVoteDiscuss*		m_pDiscuss;
	VOTEMGR_ERROR		m_nLastDiscussError;

protected:
	bool CheckDiscuss();
	void FinishDiscuss(bool bJudge);

public:
	MVoteMgr();
	virtual ~MVoteMgr();

	MVoter* FindVoter(const MUID& uid);
	void AddVoter(const MUID& uid);
	void RemoveVoter(const MUID& uid);	

	MVoteDiscuss* GetDiscuss()		{ return m_pDiscuss; }
	VOTEMGR_ERROR GetLastError()	{ return m_nLastDiscussError; }
	void SetLastError(VOTEMGR_ERROR nError)	{ m_nLastDiscussError = nError; }

	bool CallVote(MVoteDiscuss* pDiscuss);
	bool Vote(const MUID& uid, MVOTE nVote);
	void Tick(u64 nClock);

	bool IsGoingOnVote() { return (0 != m_pDiscuss); }

	void StopVote( const MUID& uidUser );
};