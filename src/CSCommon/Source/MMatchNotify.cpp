#include "stdafx.h"
#include "MMatchNotify.h"
#include "MXml.h"
#include <map>
#include "MBaseStringResManager.h"
#include "MDebug.h"

#define MESSAGES_FILE_NAME	"system/notify.xml"

#define ZTOK_MESSAGE		"NOTIFY"
#define ZTOK_ID				"id"

using MNotifyMap = std::map<int, std::string>;

static MNotifyMap g_NotifyMap;

bool InitializeNotify(MZFileSystem *pfs)
{
	MXmlDocument aXml;
	if(!aXml.LoadFromFile(MESSAGES_FILE_NAME, pfs))
	{
		MLog("InitializeNotify -- Failed to load XML from file %s\n", MESSAGES_FILE_NAME);
		return false;
	}

	auto aParent = aXml.GetDocumentElement();
	MXmlElement aChild;

	for (int i = 0, end = aParent.GetChildNodeCount(); i < end; i++)
	{
		aChild = aParent.GetChildNode(i);
		char TagName[256];
		aChild.GetTagName(TagName);
		if (_stricmp(TagName, ZTOK_MESSAGE) == 0)
		{
			int nID = 0;
			if (aChild.GetAttribute(&nID, ZTOK_ID))
			{
				_ASSERT(g_NotifyMap.find(nID) == g_NotifyMap.end());
				
				char szContents[256];
				aChild.GetContents(szContents);

				g_NotifyMap.emplace(nID, MGetStringResManager()->GetStringFromXml(szContents));
			}
		}
	}
	return true;
}

bool NotifyMessage(int nMsgID, std::string *out)
{
	auto i = g_NotifyMap.find(nMsgID);
	if (i == g_NotifyMap.end())
		return false;

	*out = i->second;

	return true;
}

bool NotifyMessage(int nMsgID, char* Output, size_t OutputSize)
{
	auto i = g_NotifyMap.find(nMsgID);
	if (i == g_NotifyMap.end())
		return false;

	strcpy_safe(Output, OutputSize, i->second.c_str());

	return true;
}