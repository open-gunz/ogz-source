#include "stdafx.h"
#include "ZNetAgreementBuilder.h"

ZNetAgreementBuilder::ZNetAgreementBuilder()
{
	Clear();
}

ZNetAgreementBuilder::~ZNetAgreementBuilder()
{
	Clear();
}

void ZNetAgreementBuilder::ClearReplies()
{
	while (!m_Replies.empty())
	{
		list<ZReplier*>::iterator itor = m_Replies.begin();
		delete *itor;
		m_Replies.erase(itor);
	}
}

void ZNetAgreementBuilder::Clear()
{
	ClearReplies();

	m_nRequestID = 0;
	m_nProposalMode = MPROPOSAL_NONE;
	m_bProposingNow = false;
}

bool ZNetAgreementBuilder::Proposal(MMatchProposalMode nProposalMode, int nRequestID, char** ppReplierNames, int nReplierCount)
{
	if (m_bProposingNow)
	{
		Clear();
	}

	m_nRequestID = nRequestID;
	m_nProposalMode = nProposalMode;
	m_bProposingNow = true;

	for (int i = 0; i < nReplierCount; i++)
	{
		ZReplier* pNewReplier = new ZReplier;
		strcpy_safe(pNewReplier->szCharName, ppReplierNames[i]);
		pNewReplier->bAnswered = false;
		pNewReplier->bAgreement = false;
		m_Replies.push_back(pNewReplier);
	}

	return true;
}

void ZNetAgreementBuilder::CancelProposal()
{
	Clear();
}

ZNetAgreementBuilder::_BuildReplyResult ZNetAgreementBuilder::BuildReply(const char* szReplierName, 
																		 const MMatchProposalMode nProposalMode, 
																		 int nRequestID, bool bAgreement)
{
	if (nRequestID != m_nRequestID) return BRR_WRONG_REPLY;
	if (nProposalMode != m_nProposalMode) return BRR_WRONG_REPLY;
	if (!m_bProposingNow) return BRR_WRONG_REPLY;
	if (m_Replies.empty()) return BRR_WRONG_REPLY;
	

	for (list<ZReplier*>::iterator itor = m_Replies.begin(); itor != m_Replies.end(); itor++)
	{
		ZReplier* pReplier = *itor;

		if (!strcmp(pReplier->szCharName, szReplierName))
		{
			pReplier->bAnswered = true;
			pReplier->bAgreement = bAgreement;
			break;
		}
	}


	for (list<ZReplier*>::iterator itor = m_Replies.begin(); itor != m_Replies.end(); itor++)
	{
		ZReplier* pReplier = *itor;
		if (!pReplier->bAnswered) 
		{
			return BRR_NOT_REPLIED_ALL;
		}
	}

	for (list<ZReplier*>::iterator itor = m_Replies.begin(); itor != m_Replies.end(); itor++)
	{
		ZReplier* pReplier = *itor;
		if (!pReplier->bAgreement) 
		{
			return BRR_DISAGREED;
		}
	}

	return BRR_ALL_AGREED;
}

int ZNetAgreementBuilder::GetReplierNames(char** ppReplierNames, size_t maxlen, size_t nMaxCount)
{
	size_t nCount{};
	for (list<ZReplier*>::iterator i=m_Replies.begin(); i!=m_Replies.end(); i++) {
		if (nCount>=nMaxCount) break;

		ZReplier* pReplier = (*i);
		strcpy_safe(ppReplierNames[nCount], maxlen, pReplier->szCharName);

		nCount++;
	}
	return static_cast<int>(nCount);
}

bool ZNetAgreementBuilder::GetRejecter(char* out, int maxlen)
{
	for (list<ZReplier*>::iterator i=m_Replies.begin(); i!=m_Replies.end(); i++) {
		ZReplier* pReplier = (*i);
		if ((pReplier->bAnswered) && (pReplier->bAgreement == false))
		{
			strcpy_safe(out, maxlen, pReplier->szCharName);
			return true;
		}
	}

	return false;
}