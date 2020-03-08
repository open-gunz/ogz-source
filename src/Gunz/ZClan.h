#pragma once

#include "ZPrerequisites.h"
#include "ZMessages.h"
#include "ZApplication.h"

inline const char* ZGetClanGradeStr(const MMatchClanGrade nGrade)
{
	switch (nGrade)
	{
	case MCG_NONE:		
		return ZMsg(MSG_WORD_CLAN_NONE); break;

	case MCG_MASTER:	
		return ZMsg(MSG_WORD_CLAN_MASTER); break;

	case MCG_ADMIN:		
		return ZMsg(MSG_WORD_CLAN_ADMIN); break;

	case MCG_MEMBER:	
		return ZMsg(MSG_WORD_CLAN_MEMBER); break;

	default:
		return "";
	}

}