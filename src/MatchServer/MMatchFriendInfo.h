#pragma once

#include <list>
#include "MMatchGlobal.h"
#include "StringView.h"

#define MAX_FRIEND_COUNT	20

struct MMatchFriendNode {
	u32	nFriendCID;
	unsigned short	nFavorite;
	char			szName[MATCHOBJECT_NAME_LENGTH];

	unsigned char	nState;
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};

using MMatchFriendList = std::list<MMatchFriendNode*>;

class MMatchFriendInfo {
private:
	MCriticalSection	m_csFriendListLock;
public:
	MMatchFriendList	m_FriendList;
public:
	MMatchFriendInfo();
	virtual ~MMatchFriendInfo();
	bool Add(u32 nFriendCID, unsigned short nFavorite, const StringView& Name);
	void Remove(const StringView& Name);
	MMatchFriendNode* Find(u32 nFriendCID);
	MMatchFriendNode* Find(const StringView& Name);
	void UpdateDesc();
};