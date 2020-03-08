#include "stdafx.h"
#include "MSafeUDP.h"
#include "MBasePacket.h"
#include <mutex>
#include "MDebug.h"
#include "MUtil.h"
#include "MFile.h"
#include "MSync.h"

#define MAX_RECVBUF_LEN		65535

#define SAFEUDP_MAX_SENDQUEUE_LENGTH		5120
#define SAFEUDP_MAX_ACKQUEUE_LENGTH			5120
#define SAFEUDP_MAX_ACKWAITQUEUE_LENGTH		64

#define SAFEUDP_SAFE_MANAGE_TIME			100		// WSA_INFINITE for debug
#define SAFEUDP_SAFE_RETRANS_TIME			500
#define SAFEUDP_MAX_SAFE_RETRANS_TIME		5000


#define LINKSTATE_LOG
void MTRACE(const char* pszLog)
{
	DMLog("%s", pszLog);
}

////////////////////////////////////////////////////////////////////////////////////////////
// MNetLink ////////////////////////////////////////////////////////////////////////////////
MNetLink::MNetLink()
{
	m_pSafeUDP = NULL;
	m_bConnected = false;
	m_nLinkState = LINKSTATE_CLOSED;
	memset((char*)&m_Address, 0, sizeof(MSocket::sockaddr_in));
	m_nNextReadIndex = 0;
	m_nNextWriteIndex = 0;
	m_dwAuthKey = 0;
	m_pUserData = NULL;

	MTime::GetTime(&m_tvConnectedTime);
	MTime::GetTime(&m_tvLastPacketRecvTime);
}

MNetLink::~MNetLink()
{
	for (ACKWaitListItor itor = m_ACKWaitQueue.begin(); itor != m_ACKWaitQueue.end(); ) {
		delete (*itor);
		itor = m_ACKWaitQueue.erase(itor);
	}
}

void MNetLink::SetLinkState(MNetLink::LINKSTATE nState) 
{ 
	if (m_nLinkState == nState)
		return;

	m_nLinkState = nState; 

	#ifdef LINKSTATE_LOG
	switch(m_nLinkState) {
	case LINKSTATE_CLOSED:
		Setconnected(false);
		MTRACE("<LINK_STATE> LINKSTATE_CLOSED \n");
		if (m_pSafeUDP && m_pSafeUDP->m_fnNetLinkStateCallback)
			m_pSafeUDP->m_fnNetLinkStateCallback(this, LINKSTATE_CLOSED);
		break;
	case LINKSTATE_ESTABLISHED:
		Setconnected(true);
		MTRACE("<LINK_STATE> LINKSTATE_ESTABLISHED \n");
		if (m_pSafeUDP && m_pSafeUDP->m_fnNetLinkStateCallback)
			m_pSafeUDP->m_fnNetLinkStateCallback(this, LINKSTATE_ESTABLISHED);
		break;
	case LINKSTATE_SYN_SENT:
		MTRACE("<LINK_STATE> LINKSTATE_SYN_SENT \n");
		break;
	case LINKSTATE_SYN_RCVD:
		MTRACE("<LINK_STATE> LINKSTATE_SYN_RCVD \n");
		break;
	case LINKSTATE_FIN_SENT:
		MTRACE("<LINK_STATE> LINKSTATE_FIN_SENT \n");
		break;
	case LINKSTATE_FIN_RCVD:
		MTRACE("<LINK_STATE> LINKSTATE_FIN_RCVD \n");
		break;
	};
	#endif
}

bool MNetLink::MakeSockAddr(const char* pszIP, int nPort, MSocket::sockaddr_in* pSockAddr)
{
	MSocket::sockaddr_in RemoteAddr{};

	// Set Dest IP and Port 
	RemoteAddr.sin_family = MSocket::AF::INET;
	RemoteAddr.sin_port = MSocket::htons(nPort);
	u32 dwAddr;
	auto pton_ret = MSocket::inet_pton(RemoteAddr.sin_family, pszIP, &dwAddr);
	if (pton_ret == 1) {
		memcpy(&(RemoteAddr.sin_addr), &dwAddr, 4);
	} else {
		auto* pHost = MSocket::gethostbyname(pszIP);
		if (pHost == NULL) {
			MLog("<SAFEUDP_ERROR> Can't resolve hostname </SAFEUDP_ERROR>\n");
			return false;
		}
		memcpy(&RemoteAddr.sin_addr, pHost->h_addr, pHost->h_length);
	}

	*pSockAddr = RemoteAddr;
	return true;
}

bool MNetLink::SetAddress(char* pszIP, int nPort)
{
	return MakeSockAddr(pszIP, nPort, &m_Address);
}

int MNetLink::GetPort()
{
	return MSocket::ntohs(m_Address.sin_port);
}

i64 MNetLink::GetMapKey()
{
	auto nKey = i64(GetRawPort());
	nKey = nKey << 32;
	nKey += GetIP();
	return nKey;
}

i64 MNetLink::GetMapKey(MSocket::sockaddr_in* pSockAddr)
{
	i64 nKey = pSockAddr->sin_port;
	nKey = nKey << 32;
	nKey += pSockAddr->sin_addr.S_un.S_addr;
	return nKey;
}

bool MNetLink::SendControl(MControlPacket::CONTROL nControl)
{
	MControlPacket* pPacket = new MControlPacket;

	if (nControl == MControlPacket::CONTROL_SYN) {	// Connect.1
		pPacket->nControl = nControl;
		SetLinkState(LINKSTATE_SYN_SENT);
	}
	else if (nControl == MControlPacket::CONTROL_FIN) {	// Disconnect.1
		pPacket->nControl = nControl;
		SetLinkState(LINKSTATE_FIN_SENT);
	}

	return m_pSafeUDP->Send(this, pPacket, sizeof(MControlPacket));
}

bool MNetLink::OnRecvControl(MControlPacket* pPacket)
{
	MControlPacket::CONTROL nControl = pPacket->nControl;
	MControlPacket* pReply = new MControlPacket;
	bool bCheckState = false;

	if (nControl == MControlPacket::CONTROL_SYN) {
		SetLinkState(LINKSTATE_SYN_RCVD);
		pReply->nControl = MControlPacket::CONTROL_SYN_RCVD;
		bCheckState = true;
	}
	else if (nControl == MControlPacket::CONTROL_FIN) {
		SetLinkState(LINKSTATE_FIN_RCVD);
		pReply->nControl = MControlPacket::CONTROL_FIN_RCVD;
		bCheckState = true;
	} else {
		switch(m_nLinkState) {
		case LINKSTATE_SYN_SENT:
			{
				if (nControl == MControlPacket::CONTROL_SYN_RCVD) {
					SetLinkState(LINKSTATE_ESTABLISHED);
					pReply->nControl = MControlPacket::CONTROL_ACK;
					bCheckState = true;
				}
			}
			break;
		case LINKSTATE_SYN_RCVD:
			{
				if (nControl == MControlPacket::CONTROL_ACK) {
					SetLinkState(LINKSTATE_ESTABLISHED);
					bCheckState = false;
				}
			}
			break;
		case LINKSTATE_FIN_SENT:
			{
				if (nControl == MControlPacket::CONTROL_FIN_RCVD) {
					SetLinkState(LINKSTATE_CLOSED);
					pReply->nControl = MControlPacket::CONTROL_ACK;
					bCheckState = true;
				}
			}
			break;
		case LINKSTATE_FIN_RCVD:
			{
				if (nControl == MControlPacket::CONTROL_ACK) {
					SetLinkState(LINKSTATE_CLOSED);
					bCheckState = false;
				}
			}
			break;
		};
	}
	if (bCheckState == false) {
		delete pReply;
		return false;
	}

	return m_pSafeUDP->Send(this, pReply, sizeof(MControlPacket));
}

bool MNetLink::SetACKWait(MSafePacket* pPacket, u32 dwPacketSize)
{
	if (m_ACKWaitQueue.size() > SAFEUDP_MAX_ACKWAITQUEUE_LENGTH)
		return false;

	pPacket->nSafeIndex = GetNextWriteIndex();
	MACKWaitItem* pACKWaitItem = new MACKWaitItem;
	pACKWaitItem->pPacket = std::unique_ptr<MSafePacket>{ pPacket };
	pACKWaitItem->dwPacketSize = dwPacketSize;
	pACKWaitItem->nSendCount = 1;		// SendQueue
	MTime::GetTime(&pACKWaitItem->tvFirstSent);
	MTime::GetTime(&pACKWaitItem->tvLastSent);
	m_ACKWaitQueue.push_back(pACKWaitItem);

	return true;
}

bool MNetLink::ClearACKWait(u8 nSafeIndex)
{
	for (ACKWaitListItor itor = m_ACKWaitQueue.begin(); itor != m_ACKWaitQueue.end(); ) {
		MACKWaitItem* pACKWaitItem = *itor;
		if (pACKWaitItem->pPacket->nSafeIndex == nSafeIndex) {
			delete pACKWaitItem;	// pACKWaitItem->pPacket will Delete too
			itor = m_ACKWaitQueue.erase(itor);
			return true;
		} else {
			++itor;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MSocketThread class /////////////////////////////////////////////////////////////////////
void MSocketThread::Create()
{
	MThread::Create(); 
}

void MSocketThread::Destroy()
{ 
	m_KillEvent.SetEvent(); 
	MThread::Destroy(); // Wait for Thread Death
}

void MSocketThread::Run()
{
	while (true)
	{
		// Check the kill event while waiting for m_pSafeUDP to be set.
		auto WaitResult = m_KillEvent.Await(100);
		if (WaitResult == MSync::WaitFailed)
		{
			LOG_SOCKET_ERROR("m_KillEvent.Await", WaitResult);
			return;
		}
		else if (WaitResult == 0)
		{
			// Kill event was triggered.
			return;
		}

		if (m_pSafeUDP)
			break;
	}

	bool bSendable = false;
	auto SocketEvent = MSocket::CreateEvent();
	MSignalEvent* EventArray[]{
		&SocketEvent,
		&m_ACKEvent,
		&m_SendEvent,
		&m_KillEvent,
	};

	{
		using namespace MSocket::FD;
		const auto Flags = READ | WRITE;
		MSocket::EventSelect(m_pSafeUDP->GetLocalSocket(), SocketEvent, Flags);
	}

	while (true)
	{
		auto WaitResult = WaitForMultipleEvents(EventArray, SAFEUDP_SAFE_MANAGE_TIME);

		if (WaitResult == MSync::WaitFailed)
		{
			LOG_SOCKET_ERROR("WaitForMultipleEvents", WaitResult);
		}

		switch (WaitResult)
		{
			// Socket Event
		case 0:
		{
			MSocket::NetworkEvents NetEvent;
			EnumNetworkEvents(m_pSafeUDP->GetLocalSocket(), SocketEvent, &NetEvent);
			if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::READ))
			{
				m_pSafeUDP->LockNetLink();
				Recv();
				m_pSafeUDP->UnlockNetLink();
			}
			if (MSocket::IsNetworkEventSet(NetEvent, MSocket::FD::WRITE))
			{
				bSendable = true;
			}
		}
			break;

			// ACK Send Event
		case 1:
			FlushACK();
			m_ACKEvent.ResetEvent();
			break;

			// Packet Send Event
		case 2:
			if (bSendable == true)
				FlushSend();
			m_SendEvent.ResetEvent();
			break;

			// Kill event
		case 3:
			DMLog("MSocketThread::Run received kill event\n");
			goto end_thread; // Stop Thread

		default:
			m_pSafeUDP->LockNetLink();
			SafeSendManage();
			m_pSafeUDP->UnlockNetLink();
			break;
		}
	}

end_thread:

	// Clear Queues
	LockSend();
	{
		for (SendListItor itor = m_SendList.begin(); itor != m_SendList.end(); ) {
			delete (*itor);
			itor = m_SendList.erase(itor);
		}
	}
	{
		for (SendListItor itor = m_TempSendList.begin(); itor != m_TempSendList.end(); ) {
			delete (*itor);
			itor = m_TempSendList.erase(itor);
		}
	}
	UnlockSend();

	LockACK();
	{
		for (ACKSendListItor itor = m_ACKSendList.begin(); itor != m_ACKSendList.end(); ) {
			delete (*itor);
			itor = m_ACKSendList.erase(itor);
		}
	}
	{
		for (ACKSendListItor itor = m_TempACKSendList.begin(); itor != m_TempACKSendList.end(); ) {
			delete (*itor);
			itor = m_TempACKSendList.erase(itor);
		}
	}
	UnlockACK();
}

bool MSocketThread::PushACK(MNetLink* pNetLink, MSafePacket* pPacket)
{
	if (m_ACKSendList.size() > SAFEUDP_MAX_ACKQUEUE_LENGTH)
		return false;

	MACKQueueItem* pACKItem = new MACKQueueItem;
	pACKItem->dwIP = pNetLink->GetIP();
	pACKItem->wRawPort = pNetLink->GetRawPort();
	pACKItem->nSafeIndex = pPacket->nSafeIndex;

	LockACK();
	m_TempACKSendList.push_back(pACKItem);
	UnlockACK();

	m_ACKEvent.SetEvent();

	return true;
}

bool MSocketThread::PushSend(MNetLink* pNetLink, MBasePacket* pPacket, u32 dwPacketSize, bool bRetransmit)
{
	if (!pNetLink || m_SendList.size() > SAFEUDP_MAX_SENDQUEUE_LENGTH)
		return false;

	MSendQueueItem* pSendItem = new MSendQueueItem;
	pSendItem->dwIP = pNetLink->GetIP();
	pSendItem->wRawPort = pNetLink->GetRawPort();
	pSendItem->pPacket = pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	if (pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != false && bRetransmit == false) {
		pNetLink->SetACKWait((MSafePacket*)pPacket, dwPacketSize);
	}

	LockSend();
	m_TempSendList.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}

bool MSocketThread::PushSend(const char* pszIP, int nPort, char* pPacket, u32 dwPacketSize)
{
	if (m_SendList.size() > SAFEUDP_MAX_SENDQUEUE_LENGTH)
		return false;

	MSocket::sockaddr_in Addr;
	if (MNetLink::MakeSockAddr(pszIP, nPort, &Addr) == false)
		return false;

	MSendQueueItem* pSendItem = new MSendQueueItem;
	pSendItem->dwIP = Addr.sin_addr.S_un.S_addr;
	pSendItem->wRawPort = Addr.sin_port;
	pSendItem->pPacket = (MBasePacket*)pPacket;
	pSendItem->dwPacketSize = dwPacketSize;

	LockSend();
	m_TempSendList.push_back(pSendItem);
	UnlockSend();

	m_SendEvent.SetEvent();

	return true;
}

bool MSocketThread::PushSend(u32 dwIP, int nPort, char* pPacket, u32 dwPacketSize)
{
	if (SAFEUDP_MAX_SENDQUEUE_LENGTH < m_SendList.size() ||
		MSocket::in_addr::None == dwIP)
	 	return false;

	MSocket::sockaddr_in Addr{};

	//	Set Dest IP and Port 
	Addr.sin_family = MSocket::AF::INET;
	Addr.sin_port = MSocket::htons(nPort);
	memcpy(&(Addr.sin_addr), &dwIP, 4);

	MSendQueueItem* pSendItem = new MSendQueueItem;
	if( 0 != pSendItem )
	{
		pSendItem->dwIP = dwIP;
		pSendItem->wRawPort = Addr.sin_port;
		pSendItem->pPacket = (MBasePacket*)pPacket;
		pSendItem->dwPacketSize = dwPacketSize;

		LockSend();
		m_TempSendList.push_back( pSendItem );
		UnlockSend();

		m_SendEvent.SetEvent();

		return true;
	}

	return false;
}

template <typename T>
bool MSocketThread::SendPacket(const T& DestAddr, const void* Data, size_t DataSize)
{
	auto SendToResult = MSocket::sendto(m_pSafeUDP->GetLocalSocket(),
		static_cast<const char*>(Data),
		DataSize,
		0,
		reinterpret_cast<const MSocket::sockaddr*>(&DestAddr),
		sizeof(DestAddr));

	if (SendToResult == MSocket::SocketError)
	{
		LOG_SOCKET_ERROR("sendto", SendToResult);
		return false;
	}
	else
	{
		m_nTotalSend += SendToResult;
		m_SendTrafficLog.Record(m_nTotalSend);
	}

	return true;
}

bool MSocketThread::FlushACK()
{
	{
		std::lock_guard<MCriticalSection> lock{ m_csACKLock };
		while (m_TempACKSendList.size() > 0) {
			ACKSendListItor itor = m_TempACKSendList.begin();
			m_ACKSendList.push_back(*itor);
			m_TempACKSendList.erase(itor);
		}
	}

	while (m_ACKSendList.size() > 0)
	{
		auto itor = m_ACKSendList.begin();
		auto* pACKItem = *itor;

		MACKPacket ACKPacket{};
		ACKPacket.nSafeIndex = pACKItem->nSafeIndex;

		MSocket::sockaddr_in DestAddr{};
		DestAddr.sin_family = MSocket::AF::INET;
		DestAddr.sin_addr.S_un.S_addr = pACKItem->dwIP;
		DestAddr.sin_port = pACKItem->wRawPort;

		SendPacket(DestAddr, &ACKPacket, sizeof(ACKPacket));

		delete pACKItem;
		m_ACKSendList.erase(itor);
	}

	return true;
}

bool MSocketThread::FlushSend()
{
	{
		std::lock_guard<MCriticalSection> lock{ m_csSendLock };
		while (m_TempSendList.size() > 0) {
			auto itor = m_TempSendList.begin();
			m_SendList.push_back(*itor);
			m_TempSendList.erase(itor);
		}
	}

	MSendQueueItem* pSendItem{};
	MSocket::sockaddr_in DestAddr{};

	while(m_SendList.size() > 0)
	{
		auto itor = m_SendList.begin();
		pSendItem = *itor;

		DestAddr.sin_family = MSocket::AF::INET;
		DestAddr.sin_addr.S_un.S_addr = pSendItem->dwIP;
		DestAddr.sin_port = pSendItem->wRawPort;

		SendPacket(DestAddr, pSendItem->pPacket, pSendItem->dwPacketSize);
		
		#ifdef _OLD_SAFEUDP
			if (pSendItem->pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != false) {
				// Don't Delete SafePacket (pSendItem->pPacket)
				delete pSendItem;
				m_SendList.erase(itor);
			} else {	// Means Normal Packet
				delete pSendItem->pPacket;
				delete pSendItem;
				m_SendList.erase(itor);
			}
		#else
			delete pSendItem->pPacket;
			delete pSendItem;
			m_SendList.erase(itor);
		#endif

	}
	return true;
}

bool MSocketThread::SafeSendManage()
{
	int nCnt = m_pSafeUDP->m_NetLinkMap.size();
	for (NetLinkItor itorLink = m_pSafeUDP->m_NetLinkMap.begin(); itorLink != m_pSafeUDP->m_NetLinkMap.end(); ) {
		MNetLink* pNetLink = (*itorLink).second;

		MTime::timeval tvNow;
		MTime::GetTime(&tvNow);

		// Closed Idle time check
		auto tvIdleDiff = MTime::TimeSub(tvNow, pNetLink->m_tvLastPacketRecvTime);
		if ( (pNetLink->GetLinkState() != MNetLink::LINKSTATE_ESTABLISHED) &&
			 ((tvIdleDiff.tv_sec*1000 + tvIdleDiff.tv_usec) > SAFEUDP_MAX_SAFE_RETRANS_TIME) ) {
			MTRACE("SUDP> Idle Control Timeout \n");
			pNetLink->SetLinkState(MNetLink::LINKSTATE_CLOSED);

			delete (*itorLink).second;
			m_pSafeUDP->m_NetLinkMap.erase(itorLink++);
			continue;
		} else {
			++itorLink;
		}

		for (MNetLink::ACKWaitListItor itorACK = pNetLink->m_ACKWaitQueue.begin(); itorACK != pNetLink->m_ACKWaitQueue.end(); ++itorACK) {
			MACKWaitItem* pACKWaitItem = *itorACK;

			auto tvDiff = MTime::TimeSub(tvNow, pACKWaitItem->tvFirstSent);
			if ((tvDiff.tv_sec*1000 + tvDiff.tv_usec) > SAFEUDP_MAX_SAFE_RETRANS_TIME) {
				// Disconnect....
				MTRACE("SUDP> Retransmit Timeout \n");
				pNetLink->SetLinkState(MNetLink::LINKSTATE_CLOSED);
				break;
			}

			tvDiff = MTime::TimeSub(tvNow, pACKWaitItem->tvLastSent);
			if ((tvDiff.tv_sec*1000 + tvDiff.tv_usec) > SAFEUDP_SAFE_RETRANS_TIME) {
				PushSend(pNetLink, pACKWaitItem->pPacket.get(), pACKWaitItem->dwPacketSize, true);
				pACKWaitItem->tvLastSent = tvNow;
				pACKWaitItem->nSendCount++;
			}
		}
	}
	return true;
}

bool MSocketThread::Recv()
{
	MSocket::sockaddr_in AddrFrom{};
	int	nAddrFromLen = sizeof(MSocket::sockaddr);
	char RecvBuf[MAX_RECVBUF_LEN];

	while (true)
	{
		const auto nRecv = MSocket::recvfrom(m_pSafeUDP->GetLocalSocket(),
			RecvBuf,
			MAX_RECVBUF_LEN,
			0, 
			reinterpret_cast<MSocket::sockaddr*>(&AddrFrom),
			&nAddrFromLen);

		if (nRecv != MSocket::SocketError) {
			m_nTotalRecv += nRecv;
			m_RecvTrafficLog.Record(m_nTotalRecv);
		}

		if (nRecv <= 0) break;

		if (m_fnCustomRecvCallback &&
			OnCustomRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, RecvBuf, nRecv) == true) {
			continue;
		} else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_CONTROL_PACKET) != false) {
			OnControlRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MBasePacket*)RecvBuf, nRecv);
			continue;
		} else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_LIGHT_PACKET) != false) {
			OnLightRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MLightPacket*)RecvBuf, nRecv);
			continue;
		} else if (((MBasePacket*)RecvBuf)->GetFlag(SAFEUDP_FLAG_ACK_PACKET) != false) {
			OnACKRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MACKPacket*)RecvBuf);
			continue;
		} else {
			OnGenericRecv(AddrFrom.sin_addr.S_un.S_addr, AddrFrom.sin_port, (MBasePacket*)RecvBuf, nRecv);
			continue;
		}
	}

	return true;
}

bool MSocketThread::OnCustomRecv(u32 dwIP, u16 wRawPort, char* pPacket, u32 dwSize)
{
	if (m_fnCustomRecvCallback)
		return m_fnCustomRecvCallback(dwIP, wRawPort, pPacket, dwSize);
	return false;
}

bool MSocketThread::OnControlRecv(u32 dwIP, u16 wRawPort, MBasePacket* pPacket, u32 dwSize)
{
	MControlPacket* pControlPacket = (MControlPacket*)pPacket;
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);

	if (pNetLink == NULL) {
		if (pControlPacket->nControl == MControlPacket::CONTROL_SYN) {
			MSocket::in_addr addr;
			addr.S_un.S_addr = dwIP;

			char ip_string[256];
			MSocket::inet_ntop(MSocket::AF::INET, &addr, ip_string, std::size(ip_string));

			auto HostOrderPort = MSocket::ntohs(wRawPort);

			pNetLink = m_pSafeUDP->OpenNetLink(ip_string, HostOrderPort);
			pNetLink->OnRecvControl(pControlPacket);
		} 
	} else {
		pNetLink->OnRecvControl(pControlPacket);
	}

	if ( pNetLink && (pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != false) )
		PushACK(pNetLink, (MSafePacket*)pPacket);

	return true;
}

bool MSocketThread::OnLightRecv(u32 dwIP, u16 wRawPort, MLightPacket* pPacket, u32 dwSize)
{
	if (m_fnLightRecvCallback)
		m_fnLightRecvCallback(dwIP, wRawPort, pPacket, dwSize);
	return true;
}

bool MSocketThread::OnACKRecv(u32 dwIP, u16 wRawPort, MACKPacket* pPacket)
{
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);
	if (pNetLink == NULL)
		return false;

	pNetLink->ClearACKWait(pPacket->nSafeIndex);

	return false;
}

bool MSocketThread::OnGenericRecv(u32 dwIP, u16 wRawPort, MBasePacket* pPacket, u32 dwSize)
{
	MNetLink* pNetLink = m_pSafeUDP->FindNetLink(dwIP, wRawPort);
	if (pNetLink == NULL)
		return false;

	if (pPacket->GetFlag(SAFEUDP_FLAG_SAFE_PACKET) != false)
		PushACK(pNetLink, (MSafePacket*)pPacket);

	// recv
	if (m_fnGenericRecvCallback)
		m_fnGenericRecvCallback(pNetLink, pPacket, dwSize);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MSafeUDP class //////////////////////////////////////////////////////////////////////////
bool MSafeUDP::Create(bool BindWinsockDLL, int Port, bool ReusePort)
{
	m_bBindWinsockDLL = BindWinsockDLL;

	if (BindWinsockDLL)
	{
		if (!MSocket::Startup())
			return false;
	}

	if (OpenSocket(Port, ReusePort) == false) {
		MLog("MSafeUDP::Create -- OpenSocket failed\n");
		return false;
	}

	m_SocketThread.SetSafeUDP(this);
	m_SocketThread.Create();
	return true;
}

void MSafeUDP::Destroy()
{
	if (m_SocketThread.GetSafeUDP() == NULL)
		return;

	DisconnectAll();

	m_SocketThread.Destroy();
	CloseSocket();

	if (m_bBindWinsockDLL) {
		MSocket::Cleanup();
		m_bBindWinsockDLL = false;
	}
}

bool MSafeUDP::OpenSocket(int nPort, bool bReuse)
{
	SOCKET sockfd = MSocket::socket(MSocket::AF::INET, MSocket::SOCK::DGRAM, 0);
	if (sockfd == MSocket::InvalidSocket)
		return false;

#ifdef _WIN32
	if (bReuse) {
		int opt = 1;
		auto setsockopt_retval = MSocket::setsockopt(sockfd, MSocket::SOL::SOCKET,
			MSocket::SO::REUSEADDR, (char*)&opt, sizeof(opt));
		if (setsockopt_retval == MSocket::SocketError)
			return false;
	}
#endif

	MSocket::sockaddr_in LocalAddress;
	LocalAddress.sin_family			= MSocket::AF::INET;
	LocalAddress.sin_addr.s_addr	= MSocket::htonl(MSocket::in_addr::Any);
	LocalAddress.sin_port			= MSocket::htons(nPort);

	const auto bind_retval = MSocket::bind(sockfd,
		(MSocket::sockaddr*)&LocalAddress, sizeof(LocalAddress));
	if (bind_retval == MSocket::SocketError) {
		MSocket::closesocket(sockfd);
		return false;
	}

	m_Socket = sockfd;
	m_LocalAddress = LocalAddress;

	return true;
}

void MSafeUDP::CloseSocket()
{
	MSocket::shutdown(m_Socket, MSocket::SD::SEND);
	MSocket::closesocket(m_Socket);
	m_Socket = 0;
}

bool MSafeUDP::Send(MNetLink* pNetLink, MBasePacket* pPacket, u32 dwPacketSize)
{
	return m_SocketThread.PushSend(pNetLink, pPacket, dwPacketSize, false);
}

bool MSafeUDP::Send(const char* pszIP, int nPort, char* pPacket, u32 dwSize)
{
	return m_SocketThread.PushSend(pszIP, nPort, pPacket, dwSize);
}

bool MSafeUDP::Send(u32 dwIP, int nPort, char* pPacket, u32 dwSize )
{
	return m_SocketThread.PushSend( dwIP, nPort, pPacket, dwSize );
}

MNetLink* MSafeUDP::FindNetLink(u32 dwIP, u16 wRawPort)
{
	i64 nKey = wRawPort;
	nKey = nKey << 32;
	nKey += dwIP;

	NetLinkItor pos = m_NetLinkMap.find(nKey);
	if (pos != m_NetLinkMap.end())
		return (*pos).second;

	return NULL;
}

MNetLink* MSafeUDP::FindNetLink(i64 nMapKey)
{
	NetLinkItor pos = m_NetLinkMap.find(nMapKey);
	if (pos != m_NetLinkMap.end())
		return (*pos).second;
	return NULL;
}

MNetLink* MSafeUDP::OpenNetLink(char* szIP, int nPort)
{
	MNetLink* pNetLink = new MNetLink;
	pNetLink->SetSafeUDP(this);
	pNetLink->SetAddress(szIP, nPort);
	
	auto nKey = pNetLink->GetMapKey();
	
	auto pos = m_NetLinkMap.find(nKey);
	if (pos != m_NetLinkMap.end()) {
		Reconnect((*pos).second);
		delete pNetLink;
		pNetLink = (*pos).second;
	} else {
		m_NetLinkMap.insert(NetLinkType(nKey, pNetLink));
	}

	return pNetLink;
}

bool MSafeUDP::CloseNetLink(MNetLink* pNetLink)
{
	auto nKey = pNetLink->GetMapKey();

	auto pos = m_NetLinkMap.find(nKey);
	if (pos == m_NetLinkMap.end())
		return false;

	delete (*pos).second;
	m_NetLinkMap.erase(pos);

	return true;
}

MNetLink* MSafeUDP::Connect(char* szIP, int nPort)
{
	auto* pNetLink = OpenNetLink(szIP, nPort);
	pNetLink->SendControl(MControlPacket::CONTROL_SYN);

	return pNetLink;
}

void MSafeUDP::Reconnect(MNetLink* pNetLink)
{
	pNetLink->SendControl(MControlPacket::CONTROL_SYN);
}

bool MSafeUDP::Disconnect(MNetLink* pNetLink)
{
	pNetLink->SendControl(MControlPacket::CONTROL_FIN);

	return true;
}

int MSafeUDP::DisconnectAll()
{
	LockNetLink();
	int nCount = 0;
	for (NetLinkItor itor = m_NetLinkMap.begin(); itor != m_NetLinkMap.end(); ) {
		delete (*itor).second;
		m_NetLinkMap.erase(itor++);
		++nCount;
	}
	UnlockNetLink();
	return nCount;
}

