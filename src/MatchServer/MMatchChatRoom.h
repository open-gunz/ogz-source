#pragma once


#include "MUID.h"
#include <map>
using namespace std;


class MCommand;


#define CHATROOM_MAX_ROOMMEMBER	64


class MMatchChatRoom {
protected:
	MUID			m_uidChatRoom;
	MUID			m_uidMaster;
	char			m_szName[128];
    MMatchObjectMap	m_PlayerList;

public:
	MMatchChatRoom(const MUID& uidRoom, const MUID& uidMaster, const char* pszName);
	virtual ~MMatchChatRoom();

	const MUID& GetUID()	{ return m_uidChatRoom; }
	const MUID& GetMaster()	{ return m_uidMaster; }
	const char* GetName()	{ return m_szName; }
	size_t GetUserCount()	{ return m_PlayerList.size(); }

	bool AddPlayer(const MUID& uidPlayer);
	void RemovePlayer(const MUID& uidPlayer);

	void RouteChat(const MUID& uidSender, char* pszMessage);
	void RouteInfo(const MUID& uidReceiver);
	void RouteCommand(const MCommand* pCommand);
};


class MMatchChatRoomMap : public map<MUID, MMatchChatRoom*> {
	MUID	m_uidGenerate;
public:
	MMatchChatRoomMap()			{	m_uidGenerate = MUID(0,10);	}
	virtual ~MMatchChatRoomMap(){}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	void Insert(const MUID& uid, MMatchChatRoom* pStage)	{	insert(value_type(uid, pStage));	}
};

class MMatchChatRoomStringSubMap : public map<string, MUID> {};


class MMatchChatRoomMgr {
protected:
	MMatchChatRoomMap			m_RoomMap;
	MMatchChatRoomStringSubMap	m_RoomStringSubMap;

public:
	MMatchChatRoomMgr();
	virtual ~MMatchChatRoomMgr();

	MMatchChatRoom* AddChatRoom(const MUID& uidMaster, const char* pszName);
	void RemoveChatRoom(const MUID& uidChatRoom);

	MMatchChatRoom* FindChatRoom(const MUID& uidChatRoom);
	MMatchChatRoom* FindChatRoomByName(const char* pszName);

	void Update();
};
