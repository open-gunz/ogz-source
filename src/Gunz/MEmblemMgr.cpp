#include "stdafx.h"
#include "MXml.h"
#include "MEmblemMgr.h"
#include "FileInfo.h"
#include <shlwapi.h>
#include "ZFilePath.h"
#include "ZPost.h"
#include "MUrl.h"

#define MTOK_EMBLEM_CLID			"CLID"
#define MTOK_EMBLEM					"EMBLEM"
#define MTOK_EMBLEM_URL				"URL"
#define MTOK_EMBLEM_CHECKSUM		"CHECKSUM"
#define MTOK_EMBLEM_TIMELASTUSED	"TIMELASTUSED"

#define MTICK_EMBLEM_SAVE_THRESHOLD		5000


bool MEmblemMgr::InitDefaut()
{
	m_szEmblemBaseDir = GetMyDocumentsPath();
	m_szEmblemBaseDir += GUNZ_FOLDER;
	m_szEmblemBaseDir += MPATH_EMBLEMFOLDER;

	m_szEmblemDataFile = std::string{}.assign(m_szEmblemBaseDir);
	m_szEmblemDataFile += MPATH_EMBLEMFILE;

	MakePath(m_szEmblemDataFile.c_str());

	return true;
}

void MEmblemMgr::Create()
{
	InitDefaut();

	m_nTotalRequest = 0;
	m_nCachedRequest = 0;

	m_HttpSpooler.SetBasePath(GetEmblemBaseDir().c_str());
	m_HttpSpooler.Create();
}

void MEmblemMgr::Destroy()
{
	if (CheckSaveFlag())
		SaveCache();

	ClearCache();
	m_HttpSpooler.Destroy();
}

bool MEmblemMgr::CreateCache()
{
	MXmlDocument	xmlDoc;

	xmlDoc.Create();
	bool bResult = xmlDoc.SaveToFile(GetEmblemDataFile().c_str());
	xmlDoc.Destroy();

	return bResult;
}

bool MEmblemMgr::LoadCache()
{
	MXmlDocument	xmlDoc;
	xmlDoc.Create();

	if (!xmlDoc.LoadFromFile(GetEmblemDataFile().c_str()))
	{
		xmlDoc.Destroy();
		return false;
	}

	MXmlElement rootElement,emblemElement,childElement;
	char szTagName[256];

	rootElement = xmlDoc.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		emblemElement = rootElement.GetChildNode(i);
		emblemElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_EMBLEM))
		{
			int nCLID = -1;
			char szURL[256]="";
			int nChecksum = -1;
			time_t tmLastUsed = 0;

			int nEmblemChildCount = emblemElement.GetChildNodeCount();

			MXmlElement chrElement;
			for (int j = 0; j < nEmblemChildCount; j++)
			{
				chrElement = emblemElement.GetChildNode(j);
				chrElement.GetTagName(szTagName);
				if (szTagName[0] == '#') continue;

				if (!strcmp(szTagName, MTOK_EMBLEM_CLID))
				{
					chrElement.GetContents(&nCLID);
				}
				else if (!strcmp(szTagName, MTOK_EMBLEM_URL))
				{
					chrElement.GetContents(szURL);
				}
				else if (!strcmp(szTagName, MTOK_EMBLEM_CHECKSUM))
				{
					chrElement.GetContents(&nChecksum);
				}
			}

			if ((nCLID != -1) && (szURL[0] != 0) && (nChecksum != -1))
			{
				emblemElement.GetChildContents((int*)&tmLastUsed, MTOK_EMBLEM_TIMELASTUSED); // 없어도 로드되게
				RegisterEmblem(nCLID, szURL, nChecksum, tmLastUsed);
			}
		}
	}

	xmlDoc.Destroy();
	return true;
}

bool MEmblemMgr::PrepareCache()
{
	if (LoadCache()) {
		return true;
	} else {
		return CreateCache();
	}
}


static bool CompareEmblem(MEmblemNode* left, MEmblemNode* right)
{
	double fDiff = difftime(left->GetTimeLastUsed(), right->GetTimeLastUsed());

	return 0.0 < fDiff;
}

bool MEmblemMgr::SaveCache()
{
	list<MEmblemNode*> sortedQueue;
	for (MEmblemMap::iterator i=m_EmblemMap.begin(); i!=m_EmblemMap.end(); i++) {
		MEmblemNode* pNode = (*i).second;
		sortedQueue.push_back(pNode);
	}
	sortedQueue.sort(CompareEmblem);

	MXmlDocument	xmlDoc;
	char szBuf[256]="";

	xmlDoc.Create();
	xmlDoc.CreateProcessingInstruction();

	MXmlElement	rootElement;

	rootElement=xmlDoc.CreateElement("XML");

	xmlDoc.AppendChild(rootElement);

	int nCount = 0;
	for (list<MEmblemNode*>::iterator i=sortedQueue.begin(); i!=sortedQueue.end(); i++) {
		MEmblemNode* pNode = (*i);

		rootElement.AppendText("\n\t");

		MXmlElement	emblemElement = rootElement.CreateChildElement(MTOK_EMBLEM);
		emblemElement.AppendText("\n\t\t");

		MXmlElement	childElement;

		sprintf_safe(szBuf,"%u", pNode->GetCLID());
		childElement = emblemElement.CreateChildElement(MTOK_EMBLEM_CLID);
		childElement.SetContents(szBuf);

		emblemElement.AppendText("\n\t\t");

		childElement = emblemElement.CreateChildElement(MTOK_EMBLEM_URL);
		childElement.SetContents(pNode->GetURL());

		emblemElement.AppendText("\n\t\t");

		sprintf_safe(szBuf,"%u", pNode->GetChecksum());
		childElement = emblemElement.CreateChildElement(MTOK_EMBLEM_CHECKSUM);
		childElement.SetContents(szBuf);

		emblemElement.AppendText("\n\t\t");

		sprintf_safe(szBuf, "%u", (unsigned int)pNode->GetTimeLastUsed());
		childElement = emblemElement.CreateChildElement(MTOK_EMBLEM_TIMELASTUSED);
		childElement.SetContents(szBuf);

		emblemElement.AppendText("\n\t");

		if (++nCount >= 1000)
			break;
	}

	rootElement.AppendText("\n");

	bool bResult = xmlDoc.SaveToFile(GetEmblemDataFile().c_str());
	xmlDoc.Destroy();

	SetSaveFlag(false);

	sortedQueue.clear();

	return bResult;
}

void MEmblemMgr::ClearCache()
{
	while(!m_EmblemMap.empty()) {
		MEmblemMap::iterator itor = m_EmblemMap.begin();
		delete (*itor).second;
		m_EmblemMap.erase(itor);
	}
}

bool MEmblemMgr::GetEmblemPath(char* pszFilePath, size_t maxlen, const char* pszURL)
{
	char szFileName[256];
	MUrl::GetPath(szFileName, pszURL);

	sprintf_safe(pszFilePath, maxlen, "%s/%s", GetEmblemBaseDir(), szFileName);

	return true;
}

bool MEmblemMgr::GetEmblemPathByCLID(unsigned int nCLID, char* poutFilePath, size_t maxlen)
{
	MEmblemMap::iterator i = m_EmblemMap.find(nCLID);
	if (i==m_EmblemMap.end())
		return false;

	MEmblemNode* pEmblem = (*i).second;
	return GetEmblemPath(poutFilePath, maxlen, pEmblem->GetURL());
}

bool MEmblemMgr::CheckEmblem(unsigned int nCLID, u32 nChecksum)
{
	MEmblemMap::iterator i = m_EmblemMap.find(nCLID);
	if (i==m_EmblemMap.end()) {
		return false;
	} else {
		MEmblemNode* pEmblem = (*i).second;
		if (pEmblem->GetChecksum() == nChecksum) {
			pEmblem->UpdateTimeLastUsed();
			return true;
		} else {
			return false;
		}
	}
}

void MEmblemMgr::PostDownload(unsigned int nCLID, unsigned int nChecksum, const char* pszURL)
{
	m_HttpSpooler.Post(nCLID, nChecksum, pszURL);
}

bool MEmblemMgr::RegisterEmblem(unsigned int nCLID, const char* pszURL, u32 nChecksum, time_t tmLastUsed)
{
	MEmblemMap::iterator i = m_EmblemMap.find(nCLID);
	if (i!=m_EmblemMap.end()) {
		delete (*i).second;
		m_EmblemMap.erase(i);
	}

	char szFilePath[_MAX_DIR]="";
	if (!GetEmblemPath(szFilePath, pszURL))
		return false;

	MEmblemNode* pEmblem = new MEmblemNode();
	pEmblem->SetCLID(nCLID);
	pEmblem->SetURL(pszURL);
	pEmblem->SetChecksum(nChecksum);
	pEmblem->SetTimeLastUsed(tmLastUsed);

	m_EmblemMap.insert(MEmblemMap::value_type(nCLID, pEmblem));

	return true;
}

void MEmblemMgr::NotifyDownloadDone(unsigned int nCLID, const char* pszURL)
{
	char szPath[_MAX_DIR]="";
	GetEmblemPathByCLID(nCLID, szPath);

	ZPostClanEmblemReady(nCLID, const_cast<char*>(pszURL));
}

bool MEmblemMgr::ProcessEmblem(unsigned int nCLID, const char* pszURL, u32 nChecksum)
{
	m_nTotalRequest++;

	if (CheckEmblem(nCLID, nChecksum)) {
		m_nCachedRequest++;
		return true;
	} else {
		PostDownload(nCLID, nChecksum, pszURL);
		return false;
	}
}

void MEmblemMgr::Tick(u32 nTick)
{
	int nRegisterCount = 0;

	unsigned int nCLID = 0;
	unsigned int nChecksum = 0;
	string strURL;

	while (m_HttpSpooler.Pop(&nCLID, &nChecksum, &strURL)) {
		char szFilePath[_MAX_DIR] = "";
		if (GetEmblemPath(szFilePath, strURL.c_str()) == false)
			return;

		//u32 nChecksum = GetFileCheckSum(szFilePath);
		if (RegisterEmblem(nCLID, strURL.c_str(), nChecksum)) {
			nRegisterCount++;
			CheckEmblem(nCLID, nChecksum);	// LastUsedTime 업데이트위해 공체크
			NotifyDownloadDone(nCLID, strURL.c_str());
		}
	}
	if (nRegisterCount > 0) {
		SetSaveFlag(true);
		SetLastSavedTick(nTick);
	}
/*	AUTOSAVE 봉인
	if (CheckSaveFlag() && (nTick - GetLastSavedTick() > MTICK_EMBLEM_SAVE_THRESHOLD))
	{
		SaveCache();
		SetLastSavedTick(nTick);

		// Logs
		char szLog[128];
		sprintf_safe(szLog, "EmblemCache> Cached Emblem : %d/%d \n",
				GetCachedRequest(), GetTotalRequest());
		OutputDebugString(szLog);
	} */
}
