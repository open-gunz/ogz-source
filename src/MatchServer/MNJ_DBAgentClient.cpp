#include "stdafx.h"
#include "MNJ_DBAgentClient.h"
#include "MErrorTable.h"

MNJ_DBAgentClient::MNJ_DBAgentClient(int nGameCode, int nServerCode) 
		: MCustomClient(), m_bConnected(false), m_nGameCode(nGameCode), m_nServerCode(nServerCode), m_nQueueTop(0)
{
	memset(m_cPacketBuf, 0, sizeof(m_cPacketBuf));
}

MNJ_DBAgentClient::~MNJ_DBAgentClient()
{
}

bool MNJ_DBAgentClient::OnSockConnect(SOCKET sock)
{
	m_bConnected = true;

	NJ_PACKET* pNewPacket = new NJ_PACKET;
	memset(pNewPacket, 0, sizeof(NJ_PACKET));

	pNewPacket->nCMD = NJ_CMD_INIT_INFO;
	pNewPacket->nDataSize = 8;

	int nCode[2];
	nCode[0] = m_nGameCode;
	nCode[1] = m_nServerCode;

	memcpy(pNewPacket->cDataBody, nCode, sizeof(int)*2);
	MCustomClient::Send((char*)pNewPacket, sizeof(NJ_PACKET));

	return true;
}

bool MNJ_DBAgentClient::OnSockDisconnect(SOCKET sock)
{
	m_bConnected = false;
	return true;
}

void MNJ_DBAgentClient::Send(const MUID& uidComm, const char* szCN, const char* szPW, bool bFreeLoginIP, u32 nChecksumPack, int nTotalUserCount)
{
	// 풀에 넣어놓는다.
	LockPool();  //-------------------------------------------------------|
	m_Pool.Insert(szCN, uidComm, nChecksumPack, bFreeLoginIP);
	UnlockPool(); //------------------------------------------------------|

	NJ_PACKET* pNewPacket = new NJ_PACKET;
	memset(pNewPacket, 0, sizeof(NJ_PACKET));

	NJ_USERINFO* pUserInfo = (NJ_USERINFO*)(pNewPacket->cDataBody);

	pNewPacket->nCMD = NJ_CMD_LOGIN_C;
	pNewPacket->nDataSize = sizeof(NJ_USERINFO);
	strcpy_safe(pUserInfo->szCN, szCN);
	strcpy_safe(pUserInfo->szPW, szPW);
	pUserInfo->m_nTotalUserCount = nTotalUserCount;

	MCustomClient::Send((char*)pNewPacket, sizeof(NJ_PACKET));
}


bool MNJ_DBAgentClient::OnSockRecv(SOCKET sock, char* pPacket, u32 dwSize)
{
	if (((m_nQueueTop + dwSize) >= NJ_QUE_SIZE) || (dwSize <= 0))
	{
		_ASSERT(0);
		return false;
	}

	memcpy(m_cPacketBuf + m_nQueueTop, pPacket, dwSize);
	m_nQueueTop += dwSize;

	while (m_nQueueTop >= sizeof(NJ_PACKET))
	{
		NJ_PACKET* pNJPacket = (NJ_PACKET*)(m_cPacketBuf);
		OnRecvPacket(pNJPacket);

		if (m_nQueueTop-sizeof(NJ_PACKET) > 0)
		{
			memcpy(m_cPacketBuf, m_cPacketBuf+sizeof(NJ_PACKET), m_nQueueTop-sizeof(NJ_PACKET));
		}
		m_nQueueTop -= sizeof(NJ_PACKET);
	}

	return true;
}


void MNJ_DBAgentClient::OnRecvPacket(NJ_PACKET *pPacket)
{
	NJ_USERINFO* pU = (NJ_USERINFO*)pPacket->cDataBody;
	
	bool bExist = false;
	MUID uidComm;
	bool bFreeLoginIP;
	u32 nChecksumPack;

	LockPool(); //------------------------------------------------------|

	MDBAgentPoolNode* pPoolNode = m_Pool.GetNode(pU->szCN);
	if (pPoolNode)
	{
		bExist = true;
		uidComm = pPoolNode->uidComm;
		bFreeLoginIP = pPoolNode->bFreeLoginIP;
		nChecksumPack = pPoolNode->nChecksumPack;
	}

	m_Pool.Remove(pU->szCN);

	UnlockPool(); //----------------------------------------------------|

	if (bExist == false) return;

	switch( pPacket->nCMD )
	{
	case NJ_CMD_LOGIN_OK:
		{
			//=====================================================================================

			MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_LOGIN_FROM_DBAGENT, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidComm));
			pNew->AddParameter(new MCommandParameterString(pU->szCN));
			pNew->AddParameter(new MCommandParameterString(pU->szNN));
			pNew->AddParameter(new MCommandParameterInt((int)pU->sSex));
			pNew->AddParameter(new MCommandParameterBool(bFreeLoginIP));
			pNew->AddParameter(new MCommandParameterUInt(nChecksumPack));
			MMatchServer::GetInstance()->PostSafeQueue(pNew);

			//=====================================================================================

		} break;

	case NJ_CMD_LOGIN_DENY_SERVERBUSY:
	case NJ_CMD_LOGIN_DENY_PASSERR:
	case NJ_CMD_LOGIN_DENY_USINGID:
	case NJ_CMD_LOGIN_DENY_CANT:
		{
			MCommand* pNew = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_LOGIN_FROM_DBAGENT_FAILED, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidComm));
			pNew->AddParameter(new MCommandParameterInt(MERR_FAILED_AUTHENTICATION));
			MMatchServer::GetInstance()->PostSafeQueue(pNew);
		} break;
	}


	

}