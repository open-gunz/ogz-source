#ifndef _MASYNCDBJOB_FRIENDLIST_H
#define _MASYNCDBJOB_FRIENDLIST_H


#include "MAsyncDBJob.h"



class MAsyncDBJob_FriendList : public MAsyncJob {
protected:
	MUID			m_uid;
	
protected:	// Input Argument
	int				m_nCID;
protected:	// Output Result
	MMatchFriendInfo*	m_pFriendInfo;
public:
	MAsyncDBJob_FriendList(const MUID& uid, int nCID)
		: MAsyncJob(MASYNCJOB_FRIENDLIST)
	{
		m_pFriendInfo = NULL;
		m_uid = uid;
		m_nCID = nCID;
	}
	virtual ~MAsyncDBJob_FriendList()	{}

	const MUID& GetUID()			{ return m_uid; }
	MMatchFriendInfo* GetFriendInfo()			{ return m_pFriendInfo; }
	void SetFriendInfo(MMatchFriendInfo* pFriendInfo)	{ m_pFriendInfo = pFriendInfo; }

	virtual void Run(void* pContext);
};





#endif