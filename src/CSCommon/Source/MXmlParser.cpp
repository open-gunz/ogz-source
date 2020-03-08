#include "stdafx.h"
#include "MXmlParser.h"
#include "MZFileSystem.h"
#include "MXml.h"
#include "MDebug.h"

bool MXmlParser::ReadXml(const char* szFileName)
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

		ParseRoot(szTagName, &chrElement);
	}

	xmlIniData.Destroy();
	return true;
}

bool MXmlParser::ReadXml(MZFileSystem* pFileSystem, const char* szFileName)
{
	MXmlDocument xmlIniData;
	if(!xmlIniData.LoadFromFile(szFileName, pFileSystem)) {
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

		ParseRoot(szTagName, &chrElement);
	}

	xmlIniData.Destroy();

	return true;
}
