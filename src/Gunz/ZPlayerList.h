#pragma once

#include "MUID.h"
#include "MMatchGlobal.h"

struct ZChannelPlayerListNode
{
	MUID				uidPlayer;
	MMatchUserGradeID	nGradeID;
	MMatchPlace			nPlace;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				szClanName[CLAN_NAME_LENGTH];
	int					nLevel;
};

struct ZClanMemberListNode
{
	MUID				uidPlayer;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	int					nLevel;
	MMatchClanGrade		nClanGrade;
	MMatchPlace			nPlace;
};

struct ZFriendListNode
{
	unsigned char	nState;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};