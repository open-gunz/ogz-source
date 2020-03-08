#ifndef _ZNETREPOSITORY_H
#define _ZNETREPOSITORY_H

#include "ZPlayerList.h"
#include "ZMyInfo.h"

class ZNetRepository
{
private:
//	ZMyInfo		m_MyInfo;
	MTD_ClanInfo	m_ClanInfo;
public:
	ZNetRepository();
	virtual ~ZNetRepository();
	static ZNetRepository* GetInstance();
public:
	// Serialize ¾¾¸®Áî

public:
	// Get ¾¾¸®Áî
//	ZMyInfo* GetMyInfo()	{ return &m_MyInfo; }

	MTD_ClanInfo *GetClanInfo() { return &m_ClanInfo; }
};


inline ZNetRepository* ZGetNetRepository()		{ return ZNetRepository::GetInstance(); }
#define	ZNR ZGetNetRepository



//inline ZMyInfo* ZGetMyInfo() { return ZNR()->GetMyInfo(); }







#endif