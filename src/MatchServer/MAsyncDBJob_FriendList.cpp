#include "stdafx.h"
#include "MAsyncDBJob_FriendList.h"

void MAsyncDBJob_FriendList::Run(void* pContext)
{
	_ASSERT(m_pFriendInfo);

	auto* pDBMgr = static_cast<IDatabase*>(pContext);

	// 해당캐릭터의 친구목록 가져오기
	if (!pDBMgr->FriendGetList(m_nCID, m_pFriendInfo)) 
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}
