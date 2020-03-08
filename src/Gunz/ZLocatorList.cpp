#include "StdAfx.h"
#include "ZLocatorList.h"
#include "MZFileSystem.h"
#include "Config.h"
#ifdef LOAD_LOCATOR_FROM_CONFIG_XML
#include "ZConfiguration.h"
#endif

bool ZLocatorList::ParseLocatorList(MXmlElement& element)
{
#ifndef LOAD_LOCATOR_FROM_CONFIG_XML
	int iCount = element.GetChildNodeCount();
	MXmlElement chrElement;
	char szTagName[256];

	for (int i = 0; i < iCount; i++)
	{
		chrElement = element.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!_stricmp(szTagName, "LOCATOR"))
		{
			ParseLocator(chrElement);
		}
	}
	return true;
#else
	m_LocatorIPList.push_back(ZGetConfiguration()->GetServerIP());
	return true;
#endif
}

bool ZLocatorList::ParseLocator(MXmlElement& element)
{
	constexpr size_t MAX_IP_LEN = 64;

	int n{};
	char ip[MAX_IP_LEN]{};
	element.GetAttribute(&n, "id");
	element.GetAttribute(ip, "IP");

#ifdef _DEBUG
	char szDbgInfo[1024];

	sprintf_safe(szDbgInfo, "Locator id:%d ip:%s\n", n, ip);
	OutputDebugString(szDbgInfo);
#endif	

	m_LocatorIPList.push_back(ip);

	return true;
}