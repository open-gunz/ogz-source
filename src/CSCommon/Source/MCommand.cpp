#include "stdafx.h"
#include "MCommand.h"
#include "MCommandManager.h"
#include "MDebug.h"

MCommandDesc::MCommandDesc(int nID, const char* szName, const char* szDescription, int nFlag)
{
	m_nID = nID;
	strcpy_safe(m_szName, szName);
	strcpy_safe(m_szDescription, szDescription);
	m_nFlag = nFlag;
}

MCommandDesc::~MCommandDesc(void)
{
	for(int i=0; i<(int)m_ParamDescs.size(); i++){
		delete m_ParamDescs[i];
	}
	m_ParamDescs.clear();
}

void MCommandDesc::AddParamDesc(MCommandParameterDesc* pParamDesc)
{
	m_ParamDescs.push_back(pParamDesc);
}

bool MCommandDesc::IsFlag(int nFlag) const
{
	return ((m_nFlag&nFlag)==nFlag);
}

MCommandDesc* MCommandDesc::Clone()
{
	MCommandDesc* pNewDesc = new MCommandDesc(m_nID, m_szName, m_szDescription, m_nFlag);
	for (int i = 0; i < GetParameterDescCount(); i++)
	{
		MCommandParameterDesc* pSrcParamDesc = GetParameterDesc(i);
		MCommandParameterDesc* pParamDesc = new MCommandParameterDesc(pSrcParamDesc->GetType(), (char*)pSrcParamDesc->GetDescription());
		pNewDesc->AddParamDesc(pSrcParamDesc);
	}
	
	return pNewDesc;
}
////////////////////////////////////////////////////////////////////////
void MCommand::Reset(void)
{
	m_pCommandDesc = NULL;
	m_nSerialNumber = 0;
	m_Sender.SetZero();
	m_Receiver.SetZero();
	ClearParam();
}

void MCommand::ClearParam(void)
{
	const int nParamCount = GetParameterCount();
	for(int i=0; i<nParamCount; ++i){
		delete m_Params[i];
	}
	m_Params.clear();
}

void MCommand::ClearParam(int i)
{
	_ASSERT(GetParameterCount() >= i);
	delete m_Params[i];
	m_Params.erase(m_Params.begin() + i);
}

MCommand::MCommand(void)
{
	Reset();
}

MCommand::MCommand(const MCommandDesc* pCommandDesc, MUID Receiver, MUID Sender)
{
	Reset();
	SetID(pCommandDesc);
	m_Receiver = Receiver;
	m_Sender = Sender;
}

MCommand::MCommand(int nID, MUID Sender, MUID Receiver, MCommandManager* pCommandManager)
{
	Reset();
	SetID(nID, pCommandManager);
	m_Sender = Sender;
	m_Receiver = Receiver;
}


MCommand::~MCommand(void)
{
	ClearParam();
}

void MCommand::SetID(const MCommandDesc* pCommandDesc)
{
	m_pCommandDesc = pCommandDesc;
	_ASSERT(m_pCommandDesc!=NULL);
	m_Params.reserve(pCommandDesc->GetParameterDescCount());	// 공간 확보
}


void MCommand::SetID(int nID, MCommandManager* pCommandManager)
{
	m_pCommandDesc = pCommandManager->GetCommandDescByID(nID);
	_ASSERT(m_pCommandDesc!=NULL);
	m_Params.reserve(m_pCommandDesc->GetParameterDescCount());	// 공간 확보
}


bool MCommand::AddParameter(MCommandParameter* pParam)
{
	_ASSERT(m_Params.capacity()==m_pCommandDesc->GetParameterDescCount());	// 미리 공간이 확보되어 있어야 한다.

	int nCount = (int)m_Params.size();
	int nParamDescCount = m_pCommandDesc->GetParameterDescCount();

	_ASSERT(nCount<nParamDescCount);				// Debug Mode 에서는 Assert로 엄격하게 체크한다.
	if(nCount>=nParamDescCount) return false;

	MCommandParameterDesc* pParamDesc = m_pCommandDesc->GetParameterDesc(nCount);
	_ASSERT(pParam->GetType()==pParamDesc->GetType());	// 명시된 파라미터여야 한다.
	if(pParam->GetType()!=pParamDesc->GetType()) return false;

	m_Params.push_back(pParam);

	return true;
}

int MCommand::GetParameterCount(void) const
{
	return (int)m_Params.size();
}

MCommandParameter* MCommand::GetParameter(int i) const
{
	if(i<0 || i>=(int)m_Params.size()) return NULL;

	return m_Params[i];
}

bool MCommand::GetParameter(void* pValue, int i, MCommandParameterType t, int nBufferSize) const
{
	if( 0 == pValue ) return false;

	MCommandParameter* pParam = GetParameter(i);
	if (pParam == NULL) return false;

	if (pParam->GetType() != t) return false;

#ifdef _DEBUG
	// 스트링과 blob 은 버퍼오버플로우 체크를 할 필요가 있다.
	if(pParam->GetType()==MPT_STR && nBufferSize < 0 ) {
		// string 파라미터는 꼭 buffer size를 적어주세요
		_ASSERT(FALSE);
	}
#endif

	if(pParam->GetType()==MPT_STR && nBufferSize>=0 ) {
		char *szParamString = *(char**)pParam->GetPointer();
		if( 0 == szParamString )
		{
			ASSERT( 0 && "NULL 포인터 스트링" );
			strcpy_safe( (char*)pValue, nBufferSize, "\0" );
			return true;
		}

		int nLength = (int)strlen(szParamString);
		if(nLength>=nBufferSize-1) {
//			mlog("buffer overflow command id %d, sender uid(%d,%d)\n", GetID(), m_Sender.High, m_Sender.Low);
			strncpy_safe((char*)pValue, nBufferSize, szParamString, nBufferSize - 2);
			((char*)pValue)[nBufferSize-1]=0;
		}else{
			pParam->GetValue(pValue);
		}
	}else {
		pParam->GetValue(pValue);
	}

	return true;
}

MCommand* MCommand::Clone(void) const
{
	if(m_pCommandDesc==NULL) return NULL;
	MCommand* pClone = new MCommand(m_pCommandDesc, m_Receiver, m_Sender);
	if( 0 == pClone ) return NULL;
	const int nParamCount = GetParameterCount();
	for(int i=0; i<nParamCount; ++i){
		MCommandParameter* pParameter = GetParameter(i);
		if(pClone->AddParameter(pParameter->Clone())==false){
			delete pClone;
			return NULL;
		}
	}	

	return pClone;
}

bool MCommand::CheckRule(void)
{
	_ASSERT(m_pCommandDesc!=NULL);
	if(m_pCommandDesc==NULL) return false;

	int nCount = GetParameterCount();
	if(nCount!=m_pCommandDesc->GetParameterDescCount()) return false;

	for(int i=0; i<nCount; ++i){
		MCommandParameter* pParam = GetParameter(i);
		MCommandParameterDesc* pParamDesc = m_pCommandDesc->GetParameterDesc(i);
		if(pParam->GetType()!=pParamDesc->GetType()) return false;

		// 제약조건 체크
		if (pParamDesc->HasConditions())
		{
			for (int j = 0; j < pParamDesc->GetConditionCount(); j++)
			{
				MCommandParamCondition* pCondition = pParamDesc->GetCondition(j);
				if (!pCondition->Check(pParam)) 
				{
					mlog("Cmd Param Condition Check Error(CMID = %d)\n", m_pCommandDesc->GetID());
					return false;
				}
			}
		}
	}

	return true;
}

int MCommand::GetData(char* pData, int nSize) const
{
	if(m_pCommandDesc==NULL) return 0;

	int nParamCount = GetParameterCount();

	unsigned short int nDataCount = sizeof(nDataCount);

	// Command id
	unsigned short int nCommandID = m_pCommandDesc->GetID();
	memcpy(pData+nDataCount, &(nCommandID), sizeof(nCommandID));
	nDataCount += sizeof(nCommandID);

	// serial number
	memcpy(pData+nDataCount, &(m_nSerialNumber), sizeof(m_nSerialNumber));
	nDataCount += sizeof(m_nSerialNumber);

	// Parameters
//	memcpy(pData+nDataCount, &nParamCount, sizeof(nParamCount));
//	nDataCount += sizeof(nParamCount);

	for(int i=0; i<nParamCount; ++i){
		MCommandParameter* pParam = GetParameter(i);
		//BYTE pt = (BYTE)pParam->GetType();
		//memcpy(pData+nDataCount, &(pt), sizeof(pt));
		//nDataCount += sizeof(pt);
		nDataCount += pParam->GetData(pData+nDataCount, nSize-nDataCount);
	}

	// Write Total Size
	memcpy(pData, &nDataCount, sizeof(nDataCount));

	return nDataCount;
}

int MCommand::GetSize() const
{
	if(m_pCommandDesc==NULL) return 0;

	int nSize = 0;

	// size + command id + serial number
	nSize = sizeof(unsigned short int) + sizeof(unsigned short int) + sizeof(m_nSerialNumber);

	int nParamCount = (int)m_Params.size();

	// Parameter Types
	//nSize += (sizeof(BYTE) * nParamCount);

	// Parameters
	for(int i=0; i<nParamCount; i++)
	{
		MCommandParameter* pParam = GetParameter(i);
		nSize += (pParam->GetSize());
	}

	return nSize;
}

#define DEFAULT_COMMAND_SNCHECKER_CAPICITY	50

MCommandSNChecker::MCommandSNChecker() : m_nCapacity(DEFAULT_COMMAND_SNCHECKER_CAPICITY)
{

}

MCommandSNChecker::~MCommandSNChecker()
{

}

void MCommandSNChecker::InitCapacity(int nCapacity)
{
	m_nCapacity = nCapacity;
	m_SNQueue.clear();
	m_SNSet.clear();
}

bool MCommandSNChecker::CheckValidate(int nSerialNumber)
{
	auto itorSet = m_SNSet.find(nSerialNumber);
	if (itorSet != m_SNSet.end())
	{
		// 중복된 커맨드이다.
		return false;
	}

	_ASSERT(m_nCapacity > 0);

	if ((int)m_SNQueue.size() >= m_nCapacity)
	{
		int nFirst = m_SNQueue.front();
		m_SNQueue.pop_front();

		m_SNSet.erase(nFirst);
	}

	m_SNSet.insert(nSerialNumber);
	m_SNQueue.push_back(nSerialNumber);

	return true;
}
