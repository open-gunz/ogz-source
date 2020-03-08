#ifndef _ZBIRDDUMMYAI_H
#define _ZBIRDDUMMYAI_H

#include "muid.h"

class ZBirdDummyClient;
class MCommand;


class ZBirdDummyAI
{
private:
protected:
	ZBirdDummyClient*	m_pClient;
	int			m_nKillCount;
	u32 m_nKillLastTime;
	bool		m_bInCombat;
	int			m_nID;

	char		m_szUserID[256];
	bool		m_bCreated;

	enum _LobbyType { ZBDAI_MASTER = 0,	ZBDAI_GUEST	};
	enum _MasterType { ZBDAI_STARTALONE = 0, ZBDAI_WAIT };
	enum _GuestType	{ ZBDAI_READY = 0, ZBDAI_FORCEDENTRY };

	_LobbyType	m_nLobbyType;
	_MasterType	m_nMasterType;
	_GuestType	m_nGuestType;

	u32	m_nLastCommandTime;
	virtual void OnRun() {}
public:
	ZBirdDummyAI();
	virtual ~ZBirdDummyAI();
	void Create(ZBirdDummyClient* pClient);
	virtual void OnCommand(MCommand* pCmd);
	void Run();
};


class ZBirdDummyAIMakeRoomFlood : public ZBirdDummyAI
{
public:
	virtual void OnRun();
	virtual void OnCommand(MCommand* pCmd);
};

class ZBirdDummyAIJoinFlood : public ZBirdDummyAI
{
public:
	ZBirdDummyAIJoinFlood();
	MUID				m_uidWantedRoom;
	u32		m_nReservedTime;
	bool				m_bReserved;
	char				m_szLastStage[256];
	virtual void OnCommand(MCommand* pCmd);
	virtual void OnRun();
};



#endif