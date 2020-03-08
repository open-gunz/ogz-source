#include "stdafx.h"
#include "MProfiler.h"

MProfiler	g_DefaultProfiler;	// Default Global Profiler

MProfileStack::~MProfileStack()
{
	while(empty()==false){
		MPROFILEITEM* pProfileItem = top();
		delete pProfileItem;
		pop();
	}
}


void MProfileLoop::AddProfile(MPROFILELOG* pPL)
{
	// 맨 뒤에서 부터 같은 Depth와 같은 이름을 가진 로그를 찾아 증가시키거나 없으면 새로 생성
	for(MProfileLoop::reverse_iterator i=rbegin(); i!=rend(); i++){
		MPROFILELOG* pLog = *i;

		if(pLog->nDepth==pPL->nDepth && strcmp(pLog->szName, pPL->szName)==0){
			pLog->nCount++;
			pLog->nMaxTime = max(pLog->nMaxTime, pPL->nMaxTime);
			pLog->nMinTime = min(pLog->nMinTime, pPL->nMinTime);
			pLog->nTotalTime += pPL->nTotalTime;
			delete pPL;
			return;
		}
	}

	pPL->nCount = 1;
	insert(begin(), pPL);
}

int MProfileLoop::GetTotalTime()
{
	int nMinDepth = 9999;
	u64 nTotalTime = 0;

	for(auto i=begin(); i!=end(); i++){
		MPROFILELOG* pLog = *i;
		nMinDepth = min(pLog->nDepth, nMinDepth);
	}

	for(auto i=begin(); i!=end(); i++){
		MPROFILELOG* pLog = *i;
		if(pLog->nDepth==nMinDepth){
			if(pLog->nTotalTime==-1){
				nMinDepth++;
				nTotalTime = 0;
				continue;
			}
			nTotalTime += pLog->nTotalTime;
		}
	}

	return static_cast<int>(nTotalTime);
}


MProfiler::MProfiler()
{
	m_pOneLoopProfile = NULL;
	m_bEnableOneLoopProfile = false;
	m_szFirstProfileName = NULL;
	m_pOneLoopProfileResult = NULL;
}
MProfiler::~MProfiler()
{
	if(m_pOneLoopProfile!=NULL){
		delete m_pOneLoopProfile;
		m_pOneLoopProfile = NULL;
	}
	if(m_pOneLoopProfileResult!=NULL){
		delete m_pOneLoopProfileResult;
		m_pOneLoopProfileResult = NULL;
	}

	if(m_szFirstProfileName!=NULL){
		delete m_szFirstProfileName;
		m_szFirstProfileName = NULL;
	}

	while(m_ProfileLoop.empty()==false){
		MPROFILELOG* pPL = *(m_ProfileLoop.begin());
		m_ProfileLoop.erase(m_ProfileLoop.begin());
		delete pPL;
	}
}

void MProfiler::BeginProfile(char* szProfileName)
{
	if(m_bEnableOneLoopProfile==false) return;

	MPROFILEITEM* pProfileItem = new MPROFILEITEM;

	strcpy_safe(pProfileItem->szName, szProfileName);

	pProfileItem->nStartTime = GetGlobalTimeMS();
	pProfileItem->nEndTime = 0;

	if(m_ProfileStack.empty()==true && m_szFirstProfileName!=NULL && strcmp(m_szFirstProfileName, szProfileName)==0){
		if(m_pOneLoopProfile!=NULL){
			if(m_pOneLoopProfileResult!=NULL) delete m_pOneLoopProfileResult;
			m_pOneLoopProfileResult = m_pOneLoopProfile;
			m_pOneLoopProfile = NULL;
		}
		delete[] m_szFirstProfileName;
		m_szFirstProfileName = NULL;
	}

	if(m_szFirstProfileName==NULL){
		int len = strlen(szProfileName) + 2;
		m_szFirstProfileName = new char[len];
		strcpy_safe(m_szFirstProfileName, len, szProfileName);
	}

	m_ProfileStack.push(pProfileItem);
}

void MProfiler::EndProfile(char* szProfileName)
{
	if(m_bEnableOneLoopProfile==false) return;

	if(m_ProfileStack.empty()==true) return;
	MPROFILEITEM* pProfileItem = m_ProfileStack.top();

	if(strcmp(pProfileItem->szName, szProfileName)!=0){
		_ASSERT(FALSE);
		return;
	}

	pProfileItem->nEndTime = GetGlobalTimeMS();
	
	MPROFILELOG* pProfileLog = new MPROFILELOG;
	strcpy_safe(pProfileLog->szName, pProfileItem->szName);
	pProfileLog->nCount = 1;
	pProfileLog->nDepth = m_ProfileStack.size()-1;
	pProfileLog->nMaxTime = pProfileItem->nEndTime - pProfileItem->nStartTime;
	pProfileLog->nMinTime = pProfileLog->nMaxTime;
	pProfileLog->nTotalTime = pProfileLog->nMaxTime;
	delete pProfileItem;

	if(m_pOneLoopProfile==NULL)
		m_pOneLoopProfile = new MProfileLoop;
	MPROFILELOG* pCopy = new MPROFILELOG;
	memcpy(pCopy, pProfileLog, sizeof(MPROFILELOG));
	m_pOneLoopProfile->AddProfile(pCopy);

	m_ProfileLoop.AddProfile(pProfileLog);

	m_ProfileStack.pop();
}

bool MProfiler::FinalAnalysis(char* szFileName)
{
	FILE* fp = fopen(szFileName, "wt");
	if(fp==NULL) return false;

	static char szLog[1024];

	int nTotalTime = GetTotalTime();
	if(nTotalTime==0) nTotalTime = 1;

	sprintf_safe(szLog, "Total Profiling Time : %8.3f sec\n", nTotalTime/1000.0f);
	fputs(szLog, fp);

	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);
	fputs("       Total        |   Count    |       Average      |         Min        |         Max        | Scope\n", fp);
	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);

	for(MProfileLoop::iterator i=m_ProfileLoop.begin(); i!=m_ProfileLoop.end(); i++){
		MPROFILELOG* pLog = *i;
		float fTotalTime = pLog->nTotalTime/1000.0f;
		float fTotalTimePercent = min(pLog->nTotalTime*100/(float)nTotalTime, 100.f);
		float fAverageTime = (pLog->nTotalTime/(float)pLog->nCount)/1000.0f;
		float fAverageTimePercent = min((pLog->nTotalTime*100/(float)pLog->nCount)/(float)nTotalTime, 100.f);
		float fMinTime = pLog->nMinTime/1000.0f;
		float fMinTimePercent = min(pLog->nMinTime*100/(float)nTotalTime, 100.f);
		float fMaxTime = pLog->nMaxTime/1000.0f;
		float fMaxTimePercent = min(pLog->nMaxTime*100/(float)nTotalTime, 100.f);
		sprintf_safe(szLog, " %8.3f (%6.2f%%) | %8d   | %8.3f (%6.2f%%) | %8.3f (%6.2f%%) | %8.3f (%6.2f%%) |",
			fTotalTime, fTotalTimePercent,
			pLog->nCount,
			fAverageTime, fAverageTimePercent,
			fMinTime, fMinTimePercent,
			fMaxTime, fMaxTimePercent);
		fputs(szLog, fp);
		for(int d=0; d<pLog->nDepth; d++) fputs("  ", fp);
		fputs(pLog->szName, fp);
		fputs("\n", fp);
	}

	fputs("----------------------------------------------------------------------------------------------------------------------------------------\n", fp);

	if(m_ProfileStack.size()>0){
		sprintf_safe(szLog, "Remained Profile Stack = %d\n", m_ProfileStack.size());
		fputs(szLog, fp);
	}

	fputs("\nGenerated by MProfiler ", fp);
	fputs("1998-2001, MAIET entertainment, Inc. all rights reserved.\n", fp);

	fclose(fp);
	return true;
}

int MProfiler::GetTotalTime()
{
	return m_ProfileLoop.GetTotalTime();
}

void MProfiler::EnableOneLoopProfile(bool bEnable)
{
	if(m_bEnableOneLoopProfile!=bEnable && m_pOneLoopProfile!=NULL){
		delete m_pOneLoopProfile;
		m_pOneLoopProfile = NULL;
	}

	m_bEnableOneLoopProfile = bEnable;
}

bool MProfiler::IsOneLoopProfile()
{
	return m_bEnableOneLoopProfile;
}

MProfileLoop* MProfiler::GetOneLoopProfile()
{
	return m_pOneLoopProfileResult;
}

MProfileLoop* MProfiler::GetProfile()
{
	return m_pOneLoopProfile;
}
