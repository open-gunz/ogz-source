#include "stdafx.h"
#include "MMatchShutdown.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"



MMatchShutdown::~MMatchShutdown()
{
	for (vector<MShutdownNotify*>::iterator i = m_ShutdownNotifyArray.begin(); i != m_ShutdownNotifyArray.end(); ++i)
	{
		delete (*i);
	}

	m_ShutdownNotifyArray.clear();
}

bool MMatchShutdown::LoadXML_ShutdownNotify(const char* pszFileName)
{
/*	m_ShutdownNotifyArray.push_back(new MShutdownNotify(0, 1000, "Shutdown Started"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(1, 1000, "5"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(2, 1000, "4"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(3, 1000, "3"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(4, 1000, "2"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(5, 1000, "1"));
	m_ShutdownNotifyArray.push_back(new MShutdownNotify(5, 1000, "Shutdown Complete"));
*/

	#define MTOK_SHUTDOWNNOTIFY		"SHUTDOWNNOTIFY"
	#define MTOK_ATTR_DELAY			"delay"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(pszFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_SHUTDOWNNOTIFY))
		{
			childElement.GetAttribute(szBuf, MTOK_ATTR_DELAY);
			int nDelay = atoi(szBuf);
			childElement.GetContents(szBuf);

			m_ShutdownNotifyArray.push_back(new MShutdownNotify(nDelay, MGetStringResManager()->GetStringFromXml(szBuf)));
		}
	}

	xmlIniData.Destroy();
	return true;
}

void MMatchShutdown::Start(u64 nClock)
{
	m_bShutdown = true;
	m_nProgressIndex = 0;
	m_nTimeLastProgress = nClock;
}

void MMatchShutdown::SetProgress(int nIndex, u64 nClock)
{
	m_nProgressIndex = nIndex;
	m_nTimeLastProgress = nClock;
}

void MMatchShutdown::Notify(int nIndex)
{
	char* pszMsg = m_ShutdownNotifyArray[nIndex]->GetString();
	MMatchServer* pServer = MMatchServer::GetInstance();
	
	MCommand* pCmd = pServer->CreateCommand(MC_ADMIN_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(MUID(0,0)));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	pCmd->AddParameter(new MCmdParamUInt(ZAAT_CHAT));
	pServer->RouteToAllClient(pCmd);

#ifdef _DEBUG
	mlog( "MMatchShutdown::Notify - Notify : %s\n", pszMsg );
#endif
}

void MMatchShutdown::Terminate()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->Shutdown();
}

void MMatchShutdown::OnRun(u64 nClock)
{
	if (IsShutdown() == false) return;

	int nIndex = GetProgressIndex();

	if (nIndex < (int)m_ShutdownNotifyArray.size()) {
		MShutdownNotify* pNotify = m_ShutdownNotifyArray[nIndex];
		if (nClock - GetTimeLastProgress() < pNotify->GetDelay())
			return;
		Notify(nIndex);
		SetProgress(nIndex+1, nClock);
	} else if (nIndex == m_ShutdownNotifyArray.size()) {
		Terminate();
		SetProgress(nIndex+1, nClock);
	} else {
		return;
	}
}
