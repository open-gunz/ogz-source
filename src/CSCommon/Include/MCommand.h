#pragma once

#include "MUID.h"

#include <vector>
#include <list>
#include <set>
#include <deque>

#include "MCommandParameter.h"
#include "MCommandManager.h"
#include "MemPool.h"
#include "GlobalTypes.h"

// Command Description Flag
#define MCDT_NOTINITIALIZED		0
#define MCDT_MACHINE2MACHINE	1
#define MCDT_LOCAL				2
#define MCDT_TICKSYNC			4
#define MCDT_TICKASYNC			8
#define MCDT_USER				16
#define MCDT_ADMIN				32
#define MCDT_PEER2PEER			64

#define MCCT_NON_ENCRYPTED		128
#define MCCT_HSHIELD_ENCRYPTED	256

#define MAX_COMMAND_PARAMS		255

class MCommandDesc{
protected:
	int			m_nID;
	char		m_szName[256];
	char		m_szDescription[256];
	int			m_nFlag;

	std::vector<MCommandParameterDesc*>	m_ParamDescs;
public:
	MCommandDesc(int nID, const char* szName, const char* szDescription, int nFlag);
	virtual ~MCommandDesc();

	void AddParamDesc(MCommandParameterDesc* pParamDesc);

	bool IsFlag(int nFlag) const;
	int GetID() const { return m_nID; }
	const char* GetName(void) const { return m_szName; }
	const char* GetDescription(void) const { return m_szDescription; }
	MCommandParameterDesc* GetParameterDesc(int i) const {
		if(i<0 || i>=(int)m_ParamDescs.size()) return NULL;
		return m_ParamDescs[i];
	}
	int GetParameterDescCount(void) const {
		return (int)m_ParamDescs.size();
	}
	MCommandParameterType GetParameterType(int i) const
	{
		if(i<0 || i>=(int)m_ParamDescs.size()) return MPT_END;
		return m_ParamDescs[i]->GetType();
	}
	MCommandDesc* Clone();
};

class MCommand : public CMemPool<MCommand> 
{
public:
	MUID						m_Sender;
	MUID						m_Receiver;
	const MCommandDesc*			m_pCommandDesc;
	std::vector<MCommandParameter*>	m_Params;
	unsigned char				m_nSerialNumber;
	void ClearParam(int i);
	void Reset();

protected:
	void ClearParam();

public:
	MCommand();
	MCommand(const MCommandDesc* pCommandDesc, MUID Receiver, MUID Sender);
	MCommand(int nID, MUID Sender, MUID Receiver, MCommandManager* pCommandManager);
	virtual ~MCommand(void);

	void SetID(const MCommandDesc* pCommandDesc);
	void SetID(int nID, MCommandManager* pCommandManager);
	int GetID() const { return m_pCommandDesc->GetID(); }
	const char* GetDescription(){ return m_pCommandDesc->GetDescription(); }

	bool AddParameter(MCommandParameter* pParam);
	int GetParameterCount(void) const;
	MCommandParameter* GetParameter(int i) const;

	bool GetParameter(void* pValue, int i, MCommandParameterType t, int nBufferSize=-1) const;

	MUID GetSenderUID(void){ return m_Sender; }
	void SetSenderUID(const MUID &uid) { m_Sender = uid; }
	MUID GetReceiverUID(void){ return m_Receiver; }

	bool IsLocalCommand(void){ return (m_Sender==m_Receiver); }

	MCommand* Clone(void) const;

	bool CheckRule();	

	int GetData(char* pData, int nSize) const;
	template <typename T = std::allocator<u8>>
	bool SetData(const char* pData, MCommandManager* pCM, unsigned short nDataLen,
		bool ReadSerial, T& Alloc);
	bool SetData(const char* pData, MCommandManager* pCM,
		unsigned short nDataLen = USHRT_MAX, bool ReadSerial = true)
	{
		std::allocator<u8> alloc;
		return SetData(pData, pCM, nDataLen, true, alloc);
	}

	int GetSize() const;
};

template <typename T>
bool MCommand::SetData(const char* pData, MCommandManager* pCM, unsigned short nDataLen,
	bool ReadSerial, T& Alloc)
{
	Reset();

	unsigned short int nDataCount = 0;

	// Get Total Size
	unsigned short nTotalSize = 0;
	memcpy(&nTotalSize, pData, sizeof(nTotalSize));

	if ((nDataLen != USHRT_MAX) && (nDataLen != nTotalSize)) return false;

	nDataCount += sizeof(nTotalSize);

	// Command
	unsigned short int nCommandID = 0;
	memcpy(&nCommandID, pData + nDataCount, sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	MCommandDesc* pDesc = pCM->GetCommandDescByID(nCommandID);
	if (pDesc == NULL)
	{
		//_ASSERT(0);
		//MLog("Unknown command ID %04X\n", nCommandID);

		return false;
	}
	SetID(pDesc);

	if (ReadSerial)
	{
		memcpy(&m_nSerialNumber, pData + nDataCount, sizeof(m_nSerialNumber));
		nDataCount += sizeof(m_nSerialNumber);
	}


	// Parameters
	int nParamCount = pDesc->GetParameterDescCount();

	for (int i = 0; i<nParamCount; ++i) {
		MCommandParameterType nParamType = pDesc->GetParameterType(i);

		MCommandParameter* pParam = NULL;
		switch (nParamType) {
		case MPT_INT:
			pParam = new MCommandParameterInt;
			break;
		case MPT_UINT:
			pParam = new MCommandParameterUInt;
			break;
		case MPT_FLOAT:
			pParam = new MCommandParameterFloat;
			break;
		case MPT_STR:
			{
				unsigned short checkSize = 0;
				memcpy(&checkSize, pData + nDataCount, sizeof(checkSize));
				if (checkSize > nDataLen || checkSize == 0)
				{
					return false;
				}
			}
			pParam = new MCommandParameterString;
			break;
		case MPT_VECTOR:
			pParam = new MCommandParameterVector;
			break;
		case MPT_POS:
			pParam = new MCommandParameterPos;
			break;
		case MPT_DIR:
			pParam = new MCommandParameterDir;
			break;
		case MPT_BOOL:
			pParam = new MCommandParameterBool;
			break;
		case MPT_COLOR:
			pParam = new MCommandParameterColor;
			break;
		case MPT_UID:
			pParam = new MCommandParameterUID;
			break;
		case MPT_BLOB:
			{
				unsigned int checkSize = 0;
				memcpy(&checkSize, pData + nDataCount, sizeof(checkSize));
				if (checkSize > nDataLen || checkSize == 0)
				{
					return false;
				}
			}
			pParam = new MCommandParameterBlob;
			break;
		case MPT_CHAR:
			pParam = new MCommandParameterChar;
			break;
		case MPT_UCHAR:
			pParam = new MCommandParameterUChar;
			break;
		case MPT_SHORT:
			pParam = new MCommandParameterShort;
			break;
		case MPT_USHORT:
			pParam = new MCommandParameterUShort;
			break;
		case MPT_INT64:
			pParam = new MCommandParameterInt64;
			break;
		case MPT_UINT64:
			pParam = new MCommandParameterUInt64;
			break;
		case MPT_SVECTOR:
			pParam = new MCommandParameterShortVector;
			break;
		default:
			//mlog("Error(MCommand::SetData): Wrong Param Type\n");
			_ASSERT(false);		// Unknow Parameter!!!
			return false;
		}

		nDataCount += pParam->SetData(pData + nDataCount);

		m_Params.push_back(pParam);

		if (nDataCount > nTotalSize)
		{
			return false;
		}
	}

	if (nDataCount != nTotalSize)
	{
		return false;
	}

	return true;
}


class MCommandSNChecker
{
private:
	int				m_nCapacity;
	std::deque<int>		m_SNQueue;
	std::set<int>		m_SNSet;
public:
	MCommandSNChecker();
	~MCommandSNChecker();
	void InitCapacity(int nCapacity);
	bool CheckValidate(int nSerialNumber);
};

#define NEWCMD(_ID)		(new MCommand(_ID))
#define MKCMD(_C, _ID)									{ _C = NEWCMD(_ID); }
#define MKCMD1(_C, _ID, _P0)							{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); }
#define MKCMD2(_C, _ID, _P0, _P1)						{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); }
#define MKCMD3(_C, _ID, _P0, _P1, _P2)					{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); }
#define MKCMD4(_C, _ID, _P0, _P1, _P2, _P3)				{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); }
#define MKCMD5(_C, _ID, _P0, _P1, _P2, _P3, _P4)		{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); _C->AddParameter(new _P4); }
#define MKCMD6(_C, _ID, _P0, _P1, _P2, _P3, _P4, _P5)	{ _C = NEWCMD(_ID); _C->AddParameter(new _P0); _C->AddParameter(new _P1); _C->AddParameter(new _P2); _C->AddParameter(new _P3); _C->AddParameter(new _P4); _C->AddParameter(new _P5); }

// Short Name
typedef MCommand				MCmd;
typedef MCommandDesc			MCmdDesc;