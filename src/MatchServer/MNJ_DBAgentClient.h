#ifndef _MNJ_DBAGENTCLIENT_H
#define _MNJ_DBAGENTCLIENT_H


#include "MCustomClient.h"
#include "MUID.h"
#include <string>
#include <map>
#include <mutex>
using namespace std;

#define NJ_QUE_SIZE (256*100)

#define	NJ_CMD_INIT_INFO              1001
#define NJ_CMD_LOGIN_C                1053
#define NJ_CMD_LOGIN_OK               1054
#define NJ_CMD_LOGIN_DENY_CANT        1601
#define NJ_CMD_SERVER_MSG             1602
#define NJ_CMD_LOGIN_DENY_SERVERBUSY  1603
#define NJ_CMD_LOGIN_DENY_PASSERR     1604
#define NJ_CMD_LOGIN_DENY_USINGID     1605
#define NJ_CMD_FAULT_LOGIN_NEXT       1606
#define NJ_CMD_LOGOUT                 1063
#define NJ_CMD_LOGOUT_OK              1064
#define NJ_CMD_BADUSER               10002

#define NJ_MAX_AVATARLAYER     26

//------------------------------------------------------------------------------------------------|

typedef struct
{
	int  nRemotUnique;

	char szCN[16]; // Unique CN code
	char szNN[16]; // User NickName
	char szPW[10]; // User PassWord

	short sSex;
	short sAvatarInfo[NJ_MAX_AVATARLAYER];
	
	int  m_nTotalUserCount; // Total User Count!

	char szDesc[62];
	char szUserNO[32];
	char m_cState;
	char szRegNum[7];
		
} NJ_USERINFO;

//------------------------------------------------------------------------------------------------|

typedef struct _NJ_PACKET
{
	int nCMD;      // #define CMD~~
	int nDataSize; // size of cDataBocy

	char cDataBody[256-8]; // USERINFO, 8 => int type * 2

} NJ_PACKET;


struct MDBAgentPoolNode
{
	MUID			uidComm;
	u32	nChecksumPack;
	bool			bFreeLoginIP;
};

class MDBAgentPool : public map<string, MDBAgentPoolNode*>
{
public:
	virtual ~MDBAgentPool()
	{
		Clear();
	}
	void Insert(string strLoginID, MUID uidComm, u32 nChecksumPack, bool bFreeLoginIP)
	{
		MDBAgentPoolNode* pNewNode = new MDBAgentPoolNode;
		pNewNode->uidComm = uidComm;
		pNewNode->nChecksumPack = nChecksumPack;
		pNewNode->bFreeLoginIP = bFreeLoginIP;

		insert(value_type(strLoginID, pNewNode));
	}
	void Remove(string strLoginID)
	{
		iterator itor = find(strLoginID);
		if (itor != end())
		{
			MDBAgentPoolNode* pNode = (*itor).second;
			delete pNode;
			erase(itor);
		}
	}
	MDBAgentPoolNode* GetNode(string strLoginID)
	{
		iterator itor = find(strLoginID);
		if (itor != end())
		{
			MDBAgentPoolNode* pNode = (*itor).second;
			return pNode;
		}

		return NULL;
	}
	void Clear()
	{
		while(begin()!=end())
		{
			delete (*begin()).second;
			erase(begin());
		}
	}
};

class MNJ_DBAgentClient : public MCustomClient
{
private:
	int					m_nGameCode;
	int					m_nServerCode;
	bool				m_bConnected;
	char				m_cPacketBuf[NJ_QUE_SIZE];
	int					m_nQueueTop;
	MDBAgentPool		m_Pool;

	std::mutex	m_csPoolLock;
	void LockPool()		{ m_csPoolLock.lock(); }
	void UnlockPool()	{ m_csPoolLock.unlock(); }

	void OnRecvPacket(NJ_PACKET *pPacket);
protected:
	virtual bool OnSockConnect(SOCKET sock);
	virtual bool OnSockDisconnect(SOCKET sock);
	virtual bool OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize);
public:
	MNJ_DBAgentClient(int nGameCode, int nServerCode);
	virtual ~MNJ_DBAgentClient();
	void Send(const MUID& uidComm, const char* szCN, const char* szPW, bool bFreeLoginIP, u32 nChecksumPack, int nTotalUserCount);
	bool IsConnected() { return m_bConnected; }
};





#endif