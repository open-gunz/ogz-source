#pragma once

#include "MCommandManager.h"
#include "MPacket.h"
#include "MDebug.h"
#include "MPacketCrypter.h"

class MCommandBuilder {	
protected:
	MUID					m_uidSender;	// client
	MUID					m_uidReceiver;	// server
	MCommandManager*		m_pCommandManager;

	#define COMMAND_BUFFER_LEN	16384

	char					m_Buffer[COMMAND_BUFFER_LEN];
	int						m_nBufferNext;

	MCommandList			m_CommandList;
	std::list<MPacketHeader*>	m_NetCmdList;

	MPacketCrypter*			m_pPacketCrypter;
	MCommandSNChecker		m_CommandSNChecker;
	bool					m_bCheckCommandSN;
protected:
	bool CheckBufferEmpty();
	bool EstimateBufferToCmd();
	void AddBuffer(char* pBuffer, int nLen);
	bool MoveBufferToFront(int nStart, int nLen);
	int MakeCommand(char* pBuffer, int nBufferLen);
	void Clear();
	int _CalcPacketSize(MPacketHeader* pPacket);
public:
	MCommandBuilder(MUID uidSender, MUID uidReceiver, MCommandManager*	pCmdMgr);
	virtual ~MCommandBuilder();
	void SetUID(MUID uidReceiver, MUID uidSender);
	void InitCrypt(MPacketCrypter* pPacketCrypter, bool bCheckCommandSerialNumber);
	bool Read(char* pBuffer, int nBufferLen);
	void SetCheckCommandSN(bool bCheck) { m_bCheckCommandSN = bCheck; }

	MCommand* GetCommand();
	MPacketHeader* GetNetCommand();
};


#ifdef _DEBUG
// Network Buffer Emulation for Debug
class MCmdBufTestor {
public:
	char	m_szBuffer[16000];
	int		m_nBufferNext;

public:
	MCmdBufTestor() { m_nBufferNext = 0; }
	int PostCmd(MCommand* pCommand) {
		const int BUF_SIZE = 1024;
		char SendBuf[BUF_SIZE];
		int nSize = pCommand->GetData(SendBuf, BUF_SIZE);
		
		int nBlockSize = nSize+sizeof(MCommandMsg);
		MCommandMsg* pMsg = (MCommandMsg*)(m_szBuffer+m_nBufferNext);
		pMsg->nMsg = MSGID_COMMAND;
		pMsg->nSize = nBlockSize;
		memcpy(pMsg->Buffer, SendBuf, nSize);

		m_nBufferNext += nBlockSize;
		return nBlockSize;
	}
};
#endif
// DEBUG SAMPLE //////////////////////////////////////////////////
//	MCOMMUNICATORINFO Info;
//	Info.uid = MUID(0,100);
//	MCommandBuilder CmdBuilder(&Info, this);
//
//	MCommand* pCmd1 = CreateCommand(MC_AGENT_CONNECT, MUID(0,0));
//	pCmd1->AddParameter(new MCmdParamStr("192.168.0.30"));
//	pCmd1->AddParameter(new MCmdParamInt(5555));
//
//	MCommand* pCmd2 = CreateCommand(MC_AGENT_CONNECT, MUID(0,0));
//	pCmd2->AddParameter(new MCmdParamStr("192.168.0.31"));
//	pCmd2->AddParameter(new MCmdParamInt(6666));
//
//	MCmdBufTestor CmdTestor;
//	int nSize1 = CmdTestor.PostCmd(pCmd1);
//	int nSize2 = CmdTestor.PostCmd(pCmd2);
//	int nSizeTotal = nSize1+nSize2;
//
//	//CmdBuilder.Read(CmdTestor.m_szBuffer, 4);
//	//CmdBuilder.Read(CmdTestor.m_szBuffer+4, 45);
//	//CmdBuilder.Read(CmdTestor.m_szBuffer+4+45, nSizeTotal-4-45);
//////////////////////////////////////////////////////////////////