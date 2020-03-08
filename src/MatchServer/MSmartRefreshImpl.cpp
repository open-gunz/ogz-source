#include "stdafx.h"
#include "MSmartRefreshImpl.h"
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MMatchChannel.h"


//// MRefreshCategoryChannel ////
bool MRefreshCategoryChannelImpl::OnUpdateChecksum(u64 nTick)
{
	u32 nChecksum = 0;
	int nPage = GetCategory();
	MChannelUserArray* pArray = GetMatchChannel()->GetUserArray();
	for (int i=0; i<pArray->GetPageSize(); i++) {
		MMatchObject* pObj = pArray->Get(nPage,i);
		if (pObj == NULL) continue;

		nChecksum += pObj->GetUID().High;
		nChecksum += pObj->GetUID().Low;
		nChecksum += pObj->GetPlace();

		if (pObj->GetCharInfo())
		{
			nChecksum += pObj->GetCharInfo()->m_ClanInfo.m_nClanID;
			nChecksum += pObj->GetCharInfo()->m_ClanInfo.m_nGrade;
		}
	}
	SetChecksum(nChecksum);
	return true;
}

//// MRefreshClientChannel ////
bool MRefreshClientChannelImpl::OnSync(u32 nChecksum)
{
	MMatchObject* pObj = GetMatchObject();
	MMatchServer::GetInstance()->ChannelResponsePlayerList(pObj->GetUID(), pObj->GetChannelUID(), GetCategory());

	return true;
}


//// MRefreshCategoryClanMember ////
bool MRefreshCategoryClanMemberImpl::OnUpdateChecksum(u64 nTick)
{
	MMatchClan* pClan = GetMatchClan();
	u32 nChecksum = 0;
	for (auto itor= pClan->GetMemberBegin(); itor != pClan->GetMemberEnd(); ++itor)
	{
		MMatchObject* pObj = itor->second;

		nChecksum += pObj->GetUID().High;
		nChecksum += pObj->GetUID().Low;
		nChecksum += pObj->GetPlace();

		if (pObj->GetCharInfo())
		{
			nChecksum += pObj->GetCharInfo()->m_ClanInfo.m_nClanID;
			nChecksum += pObj->GetCharInfo()->m_ClanInfo.m_nGrade;
		}
	}
	SetChecksum(nChecksum);
	return true;
}


//// MRefreshClientClanMemberImpl ////
bool MRefreshClientClanMemberImpl::OnSync(u32 nChecksum)
{
	MMatchObject* pObj = GetMatchObject();
	MMatchServer::GetInstance()->ResponseClanMemberList(pObj->GetUID());
	return true;
}
