#include "stdafx.h"
#include "MServer.h"
#include "MSharedCommandTable.h"
#include "MCommandBuilder.h"
#include <stdarg.h>
#include "MErrorTable.h"
#include "MCRC32.h"
#include "MMatchUtil.h"
#include "MTime.h"

static int g_LogCommObjectCreated = 0;
static int g_LogCommObjectDestroyed = 0;

extern "C" void RCPLog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf_safe(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)(strlen(szBuf)-1);
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = 0;
		strcat_safe(szBuf, "\n");
	}
	DMLog(szBuf);
}

MServer::MServer()
{
	SetName("NoName");	// For Debug
#ifdef _DEBUG
	SetupRCPLog(RCPLog);
#endif
}

MServer::~MServer() = default;

bool MServer::Create(int nPort, const bool bReuse)
{
	bool bResult = true;

	if (MCommandCommunicator::Create() == false)
	{
		mlog( "MServer::Create - MCommandCommunicator::Create()==false\n" );
		bResult = false;
	}
	if(Net.Create(nPort, {RCPCallback, this}, bReuse)==false) 
	{
		mlog( "MServer::Create - Net.Create(%u)==false", nPort );
		bResult = false;
	}

	return bResult;
}

void MServer::Destroy(void)
{
	// Log 
	RCPLog("MServer::Destroy() CommObjectCreated=%d, CommObjectDestroyed=%d \n", 
		g_LogCommObjectCreated, g_LogCommObjectDestroyed);

	Net.Destroy();

	LockCommList();
		for(auto i=m_CommRefCache.begin(); i!=m_CommRefCache.end(); i++){
			delete i->second;
		}
		m_CommRefCache.clear();
	UnlockCommList();

	MCommandCommunicator::Destroy();
}

int MServer::GetCommObjCount()	
{ 
	int count = 0;
	LockCommList();
		count = (int)m_CommRefCache.size(); 
	UnlockCommList();

	return count;
}

void MServer::AddCommObject(const MUID& uid, MCommObject* pCommObj)
{
	pCommObj->SetUID(uid);
	MCommandBuilder* pCmdBuilder = pCommObj->GetCommandBuilder();
	pCmdBuilder->SetUID(GetUID(), uid);

	m_CommRefCache.emplace(uid, pCommObj);
	g_LogCommObjectCreated++;
}

void MServer::RemoveCommObject(const MUID& uid)
{
	MCommObject* pNew = m_CommRefCache.Remove(uid);
	if (pNew) delete pNew;
	g_LogCommObjectDestroyed++;
}

void MServer::InitCryptCommObject(MCommObject* pCommObj, unsigned int nTimeStamp)
{
	MPacketCrypterKey key;

	MMakeSeedKey(&key, m_This, pCommObj->GetUID(), nTimeStamp);
	pCommObj->GetCrypter()->InitKey(&key);
	pCommObj->GetCommandBuilder()->InitCrypt(pCommObj->GetCrypter(), true);
}

void MServer::PostSafeQueue(MCommand* pNew)
{
	LockSafeCmdQueue();
		m_SafeCmdQueue.push_back(pNew);
	UnlockSafeCmdQueue();
}

void MServer::SendCommand(MCommand* pCommand)
{
	_ASSERT(pCommand->GetReceiverUID().High || pCommand->GetReceiverUID().Low);

	uintptr_t nClientKey = 0;
	bool bGetComm = false;

	MPacketCrypterKey CrypterKey;

	LockCommList();
		MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(pCommand->m_Receiver);
		if(pCommObj){
			nClientKey = pCommObj->GetUserContext();
			memcpy(&CrypterKey, pCommObj->GetCrypter()->GetKey(), sizeof(MPacketCrypterKey));
			bGetComm = true;
		}
	UnlockCommList();

	if (bGetComm == false) return;

	int nSize = pCommand->GetSize();
	if ((nSize <= 0) || (nSize >= MAX_PACKET_SIZE)) return;

	char CmdData[MAX_PACKET_SIZE];
	nSize = pCommand->GetData(CmdData, nSize);

	if(pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		SendMsgCommand(nClientKey, CmdData, nSize, MSGID_RAWCOMMAND, NULL);
	}
	else 
	{
		SendMsgCommand(nClientKey, CmdData, nSize, MSGID_COMMAND, &CrypterKey);
	}
}

void MServer::OnPrepareRun(void)
{
	LockSafeCmdQueue();
		MCommandList::iterator itorCmd;
		while ( (itorCmd = m_SafeCmdQueue.begin()) != m_SafeCmdQueue.end()) {
			MCommand* pCmd = (*itorCmd);
			m_SafeCmdQueue.pop_front();
			GetCommandManager()->Post(pCmd);
		}
	UnlockSafeCmdQueue();
}

void MServer::OnRun(void)
{

}

bool MServer::OnCommand(MCommand* pCommand)
{
	switch(pCommand->GetID()){
	case MC_LOCAL_LOGIN:
		{
			MUID uidComm, uidPlayer;
			pCommand->GetParameter(&uidComm, 0, MPT_UID);
			pCommand->GetParameter(&uidPlayer, 1, MPT_UID);
			OnLocalLogin(uidComm, uidPlayer);
			return true;
		}
		break;

	case MC_NET_ECHO:
		{
			char szMessage[256];
			if (pCommand->GetParameter(szMessage, 0, MPT_STR, sizeof(szMessage) )==false) break;
			MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_ECHO), pCommand->m_Sender, m_This);
			pNew->AddParameter(new MCommandParameterString(szMessage));
			Post(pNew);
			return true;
		}
		break;

	case MC_NET_CLEAR:
		{
			if (pCommand->GetSenderUID() != MUID(0, 2)) 
			{
				// Not sent by server
				break;
			}

			MUID uid;
			if (pCommand->GetParameter(&uid, 0, MPT_UID) == false) break;
			OnNetClear(uid);
			return true;
		}
		break;

	case MC_NET_CHECKPING:
		{
			MUID uid;
			if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;
			MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
			pNew->AddParameter(new MCommandParameterUInt(static_cast<u32>(GetGlobalTimeMS())));
			Post(pNew);
			return true;
		}
		break;

	case MC_NET_PING:
		{
			unsigned int nTimeStamp;
			if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;
			MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
			pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
			Post(pNew);
			return true;
		}
		break;

	case MC_NET_PONG:
		{
			unsigned int nTimeStamp;
			pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

			OnNetPong(pCommand->GetSenderUID(), nTimeStamp);
			return true;
		}
		break;
	};
	return false;
}

void MServer::OnNetClear(const MUID& CommUID)
{
	LockCommList();
		RemoveCommObject(CommUID);
	UnlockCommList();
}

void MServer::OnNetPong(const MUID& CommUID, unsigned int nTimeStamp)
{
}

int MServer::Connect(MCommObject* pCommObj)
{
	auto Handle = Net.Connect(pCommObj->GetIP(), pCommObj->GetPort(), pCommObj);

	// UID Caching
	LockCommList();
		AddCommObject(pCommObj->GetUID(), pCommObj);
		pCommObj->SetUserContext(Handle);
	UnlockCommList();

	return MOK;
}

int MServer::ReplyConnect(MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	if (SendMsgReplyConnect(pTargetUID, pAllocUID, nTimeStamp, pCommObj) == true)
		return MOK;
	else
		return MERR_UNKNOWN;	
}

int MServer::OnAccept(MCommObject* pCommObj)
{
	MUID AllocUID = UseUID();
	if(AllocUID.IsInvalid()){
		Log(LOG_DEBUG, "Communicator has not UID space to allocate your UID.");
		return MERR_COMMUNICATOR_HAS_NOT_UID_SPACE;
	}

	pCommObj->SetUID(AllocUID);

	LockAcceptWaitQueue();
		m_AcceptWaitQueue.push_back(pCommObj);
	UnlockAcceptWaitQueue();

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_LOCAL_LOGIN), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID(pCommObj->GetUID()));
	pNew->AddParameter(new MCommandParameterUID(MUID(0,0)));
	PostSafeQueue(pNew);

	return MOK;
}

void MServer::OnLocalLogin(MUID CommUID, MUID PlayerUID)
{
	MCommObject* pCommObj = NULL;

	LockAcceptWaitQueue();
	for (auto i = m_AcceptWaitQueue.begin(); i!= m_AcceptWaitQueue.end(); i++) {
		MCommObject* pTmpObj = (*i);
		if (pTmpObj->GetUID() == CommUID) {
			pCommObj = pTmpObj;
			m_AcceptWaitQueue.erase(i);
			break;
		}
	}
	UnlockAcceptWaitQueue();

	if (pCommObj == NULL) 
		return;

	auto nTimeStamp = static_cast<unsigned int>((GetGlobalTimeMS() * 103) - 234723);

	LockCommList();
		AddCommObject(pCommObj->GetUID(), pCommObj);
		InitCryptCommObject(pCommObj, nTimeStamp);
	UnlockCommList();

	int nResult = ReplyConnect(&m_This, &CommUID, nTimeStamp, pCommObj);
}

void MServer::Disconnect(MUID uid)
{
	uintptr_t nClientKey = 0;
	bool bGetComm = false;

	LockCommList();
		MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(uid);
		if(pCommObj){
			nClientKey = pCommObj->GetUserContext();
			bGetComm = true;
		}
	UnlockCommList();

	if (bGetComm == false) 
		return;

	OnDisconnect(uid);
	Net.Disconnect(nClientKey);
}

int MServer::OnDisconnect(const MUID& uid)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_CLEAR), m_This, m_This);
	pNew->AddParameter(new MCommandParameterUID(uid));
	PostSafeQueue(pNew);
	
	return MOK;
}

bool MServer::SendMsgReplyConnect(MUID* pHostUID, MUID* pAllocUID, unsigned int nTimeStamp, MCommObject* pCommObj)
{
	auto nKey = pCommObj->GetUserContext();
	
	MReplyConnectMsg* pMsg = (MReplyConnectMsg*)malloc(sizeof(MReplyConnectMsg));
	pMsg->nMsg = MSGID_REPLYCONNECT; 
	pMsg->nSize = sizeof(MReplyConnectMsg);
	pMsg->nHostHigh = pHostUID->High;
	pMsg->nHostLow = pHostUID->Low;
	pMsg->nAllocHigh = pAllocUID->High;
	pMsg->nAllocLow = pAllocUID->Low;
	pMsg->nTimeStamp = nTimeStamp;

	return Net.Send(nKey, pMsg, pMsg->nSize);
}

bool MServer::SendMsgCommand(uintptr_t nClientKey, char* pBuf, int nSize, unsigned short nMsgHeaderID, MPacketCrypterKey* pCrypterKey)
{
	int nBlockSize = nSize+sizeof(MPacketHeader);
	MCommandMsg* pMsg = (MCommandMsg*)malloc(nBlockSize);
	pMsg->Buffer[0] = 0;
	pMsg->nCheckSum = 0;
	pMsg->nMsg = nMsgHeaderID;;
	pMsg->nSize = nBlockSize;
	int nPacketSize = nBlockSize;

	if (nMsgHeaderID == MSGID_RAWCOMMAND)
	{
		memcpy(pMsg->Buffer, pBuf, nSize);
	}
	else if (nMsgHeaderID == MSGID_COMMAND)
	{
		if ((nSize > MAX_PACKET_SIZE) || (pCrypterKey == NULL)) return false;
		
		if (!MPacketCrypter::Encrypt((char*)&pMsg->nSize, sizeof(unsigned short), pCrypterKey))
			return false;

		char SendBuf[MAX_PACKET_SIZE];

		if (!MPacketCrypter::Encrypt(pBuf, nSize, SendBuf, nSize, pCrypterKey))
			return false;

		memcpy(pMsg->Buffer, SendBuf, nSize);			
	}
	else
	{
		_ASSERT(0);
		return false;
	}

	pMsg->nCheckSum = MBuildCheckSum(pMsg, nPacketSize);

	return Net.Send(nClientKey, pMsg, nPacketSize);
}

void MServer::RCPCallback(void* pCallbackContext, NetIO::IOOperation Op, NetIO::ConnectionHandle Handle,
	const void* Data)
{
	MServer* pServer = (MServer*)pCallbackContext;

	switch (Op)
	{
	case NetIO::IOOperation::Accept:
	{
		auto AData = static_cast<const NetIO::AcceptData*>(Data);
		MCommObject* pCommObj = new MCommObject(pServer);
		MSocket::in_addr addr;
		addr.s_addr = AData->Address;
		char IPString[64];
		GetIPv4String(addr, IPString);
		pCommObj->SetAddress(IPString, AData->Port);
		pCommObj->SetUserContext(Handle);

		pServer->OnAccept(pCommObj);
			
		pServer->Net.SetContext(Handle, pCommObj);
	}
		break;
	case NetIO::IOOperation::Disconnect:
	{
		if (auto pCommObj = static_cast<MCommObject*>(pServer->Net.GetContext(Handle)))
			pServer->OnDisconnect(pCommObj->GetUID());
	}
		break;
	case NetIO::IOOperation::Read:
	{
		auto RData = static_cast<const NetIO::ReadData*>(Data);
		auto pPacket = RData->Data.data();
		auto dwPacketLen = RData->Data.size();
		auto pCommObj = static_cast<MCommObject*>(pServer->Net.GetContext(Handle));
		if ((pCommObj) && (pCommObj->IsAllowed()))
		{
			// New Cmd Buffer ////////////////
			MCommandBuilder* pCmdBuilder = pCommObj->GetCommandBuilder();

			if (pCmdBuilder)
			{
				if (!pCmdBuilder->Read((char*)pPacket, dwPacketLen))
				{
					// 패킷이 제대로 안오면 끊어버린다.
					pCommObj->SetAllowed(false);
					pServer->Net.Disconnect(pCommObj->GetUserContext());
					return;
				}

				while (MCommand* pCmd = pCmdBuilder->GetCommand()) {
					pServer->PostSafeQueue(pCmd);
				}

				while (MPacketHeader* pNetCmd = pCmdBuilder->GetNetCommand()) {
					if (pNetCmd->nMsg == MSGID_REPLYCONNECT) {
						MReplyConnectMsg* pMsg = (MReplyConnectMsg*)pNetCmd;
						MUID HostUID, AllocUID;
						unsigned int nTimeStamp;

						HostUID.High = pMsg->nHostHigh;
						HostUID.Low = pMsg->nHostLow;
						AllocUID.High = pMsg->nAllocHigh;
						AllocUID.Low = pMsg->nAllocLow;
						nTimeStamp = pMsg->nTimeStamp;

						free(pNetCmd);

						pServer->LockCommList();
						pServer->OnConnected(&HostUID, &AllocUID, nTimeStamp, pCommObj);
						pServer->UnlockCommList();
					}
				}
			}
		}
	}
		break;
	}
}

void MServer::LogF(unsigned int Level, const char* Format, ...)
{
	char buf[512];

	va_list args;

	va_start(args, Format);
	vsprintf_safe(buf, Format, args);
	va_end(args);

	Log(Level, buf);
}
