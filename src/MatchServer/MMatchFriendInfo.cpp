#include "stdafx.h"
#include "MMatchFriendInfo.h"


MMatchFriendInfo::MMatchFriendInfo()
{


}

MMatchFriendInfo::~MMatchFriendInfo()
{
	while (m_FriendList.size() > 0) {
		MMatchFriendList::iterator i = m_FriendList.begin();
		MMatchFriendNode* pNode = (MMatchFriendNode*)(*i);
		delete pNode;
		m_FriendList.pop_front();
	}
}

bool MMatchFriendInfo::Add(u32 nFriendCID, unsigned short nFavorite, const StringView& Name)
{
	if (Find(nFriendCID) != NULL)
		return false;

	MMatchFriendNode* pNode = new MMatchFriendNode;
	pNode->nFriendCID = nFriendCID;
	pNode->nFavorite = nFavorite;
	strcpy_safe(pNode->szName, Name);
	strcpy_safe(pNode->szDescription, "");
	m_FriendList.push_back(pNode);

	return true;
}

void MMatchFriendInfo::Remove(const StringView& Name)
{
	for (auto it = m_FriendList.begin(); it != m_FriendList.end(); ++it)
	{
		auto* Friend = *it;
		if (iequals(Friend->szName, Name))
		{
			delete Friend;
			m_FriendList.erase(it);
			return;
		}
	}
}

MMatchFriendNode* MMatchFriendInfo::Find(u32 nFriendCID)
{
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		if (pNode->nFriendCID == nFriendCID)
			return pNode;
	}
	return NULL;
}

MMatchFriendNode* MMatchFriendInfo::Find(const StringView& Name)
{
	for (auto&& Friend : m_FriendList)
	{
		if (iequals(Friend->szName, Name))
		{
			return Friend;
		}
	}

	return nullptr;
}

void MMatchFriendInfo::UpdateDesc()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		pNode->szDescription[0] = 0;

		MMatchObject* pObj = pServer->GetPlayerByName(pNode->szName);
		if (pObj) {
			char szDesc[CHANNELNAME_LEN*2]="";

			pNode->nState = pObj->GetPlace();
			MMatchChannel* pChannel = pServer->FindChannel(pObj->GetChannelUID());
			if (pChannel) {
				sprintf_safe(szDesc, "Channel '%s'", pChannel->GetName());
				strcpy_safe(pNode->szDescription, szDesc);
			} else {
				strcpy_safe(pNode->szDescription, "Unknown Channel");
			}
		} else {
			pNode->nState = MMP_OUTSIDE;
			strcpy_safe(pNode->szDescription, "Not Logged on");
		}
	}
}
