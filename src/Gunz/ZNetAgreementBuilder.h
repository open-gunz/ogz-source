#pragma once

#include "MUID.h"
#include "MMatchGlobal.h"
#include <list>

class ZNetAgreementBuilder
{
private:
	int						m_nRequestID;
	MMatchProposalMode		m_nProposalMode;
	bool					m_bProposingNow;

	struct ZReplier
	{
		char	szCharName[MATCHOBJECT_NAME_LENGTH];
		bool	bAnswered;
		bool	bAgreement;
	};
	std::list<ZReplier*>			m_Replies;

	void ClearReplies();
	void Clear();
public:
	ZNetAgreementBuilder();
	virtual ~ZNetAgreementBuilder();
	
	bool Proposal(MMatchProposalMode nProposalMode, int nRequestID, char** ppReplierNames, int nReplierCount);
	void CancelProposal();

	enum _BuildReplyResult
	{
		BRR_WRONG_REPLY		= 0,
		BRR_NOT_REPLIED_ALL	= 1,
		BRR_ALL_AGREED		= 2,
		BRR_DISAGREED		= 3
	};

	ZNetAgreementBuilder::_BuildReplyResult BuildReply(const char* szReplierName,
		const MMatchProposalMode nProposalMode, int nRequestID, bool bAgreement);
	MMatchProposalMode GetProposalMode()	{ return m_nProposalMode; }
	int GetReplierNames(char** ppReplierNames, size_t maxlen, size_t nMaxCount);
	template<size_t size>
	bool GetRejecter(char(&out)[size]) {
		return GetRejecter(out, size);
	}
	bool GetRejecter(char* out, int maxlen);
};