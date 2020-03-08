/////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MMatchQuestMonsterGroup.h"
#include "MZFileSystem.h"
#include "FileInfo.h"

#define MTOK_MONSTERGROUP	"GROUP"
#define MTOK_NPC			"NPC"
#define MTOK_ATTR_ID		"id"
#define MTOK_ATTR_NAME		"name"


MNPCGroupMgr::MNPCGroupMgr()
{

}

MNPCGroupMgr::~MNPCGroupMgr()
{
	Clear();
}

MNPCGroupMgr* MNPCGroupMgr::GetInstance()
{
	static MNPCGroupMgr g_NPCGroupMgr;
	return &g_NPCGroupMgr;
}

void MNPCGroupMgr::Clear()
{
	while(!empty())	{
		MNPCGroup* pGroup = (*begin()).second;
		delete pGroup; 
		pGroup = NULL;
		erase(begin());
	}
}


MNPCGroup* MNPCGroupMgr::GetGroup(const string& strName)
{
	iterator itor = find(strName);
	if (itor != end()) {
		return (*itor).second;
	}
	return NULL;
}

MNPCGroup* MNPCGroupMgr::GetGroup(int nGroupID)
{
	iterator it;
	MNPCGroup* pGroup = NULL;

	for(it = begin();it!=end(); ++it ) {

		pGroup = (*it).second;

		if(pGroup) {
			if(pGroup->GetID()==nGroupID) {
				return pGroup;
			}
		}
	}
	return NULL;
}

bool MNPCGroupMgr::ReadXml(const char* szFileName)
{
	MXmlDocument xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName)) {
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_MONSTERGROUP))
		{
			ParseRule(&chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MNPCGroupMgr::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	char *buffer;
	MZFile mzf;

	if(pFileSystem) {
		if(!mzf.Open(szFileName,pFileSystem))  {
			if(!mzf.Open(szFileName))  {
				xmlIniData.Destroy();
				return false;
			}
		}
	} 
	else  {

		if(!mzf.Open(szFileName)) {

			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	if(!xmlIniData.LoadFromMemory(buffer)) {
		xmlIniData.Destroy();
		return false;
	}

	delete[] buffer;
	mzf.Close();


	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++) {

		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);

		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_MONSTERGROUP)) {
			ParseRule(&chrElement);
		}
	}

	xmlIniData.Destroy();
	return true;
}

void MNPCGroupMgr::ParseRule(MXmlElement* pElement)
{
	int nID = 0;
	pElement->GetAttribute(&nID, MTOK_ATTR_ID);
	char szName[128]="";
	pElement->GetAttribute(szName, MTOK_ATTR_NAME);	

	MNPCGroup* pGroup = new MNPCGroup;
	pGroup->SetID(nID);
	pGroup->SetName(szName);


	MXmlElement childElement;
	char szTagName[256]=""; char szAttr[256]="";

	int nCount = pElement->GetChildNodeCount();

	for (int i=0; i<nCount; i++) {
		childElement = pElement->GetChildNode(i);

		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, MTOK_NPC))
		{
			childElement.GetAttribute(szAttr, MTOK_ATTR_NAME);
			pGroup->AddNpc(szAttr);
		}
	}

	insert(value_type(pGroup->GetName(), pGroup));
}

