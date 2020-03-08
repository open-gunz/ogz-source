#include "stdafx.h"
#include "MXml.h"
#include "RDummyList.h"

_NAMESPACE_REALSPACE2_BEGIN

bool RDummyList::Open(rapidxml::xml_node<>& parent)
{
	for (auto* node = parent.first_node(); node; node = node->next_sibling())
	{
		auto* szTagName = node->name();
		if (!szTagName)
			continue;

		if (szTagName[0] == '#')
			continue;

		if (_stricmp(szTagName, RTOK_DUMMY) != 0)
			continue;

		auto name_attr = node->first_attribute(RTOK_NAME);
		if (!name_attr)
			continue;

		auto* name = name_attr->value();
		if (!name)
			continue;

		emplace_back();
		auto& Dummy = back();
		Dummy.Name = name;

		for (auto* prop_node = node->first_node(); prop_node; prop_node = prop_node->next_sibling())
		{
			auto* szTagName = prop_node->name();
			if (!szTagName)
				continue;
			const char* szContents = prop_node->value();
			if (!szContents)
				szContents = "";

			auto ReadVector = [&]() {
				v3 vec;
				if (sscanf(szContents, "%f %f %f", &vec.x, &vec.y, &vec.z) != 3)
					vec = { 0, 0, 0 };
				return vec;
			};

			if (_stricmp(szTagName, RTOK_POSITION) == 0)
				Dummy.Position = ReadVector();
			else if (_stricmp(szTagName, RTOK_DIRECTION) == 0)
				Dummy.Direction = ReadVector();
		}
	}

	return true;
}

bool RDummyList::Save(MXmlElement *pElement)
{
	MXmlElement	aDummyListElement = pElement->CreateChildElement(RTOK_DUMMYLIST);

	for (auto& Dummy : *this)
	{
		aDummyListElement.AppendText("\n\t\t");

		char buffer[256];

		MXmlElement aElement,aChild;
		aElement = aDummyListElement.CreateChildElement(RTOK_DUMMY);

		aElement.AddAttribute(RTOK_NAME, Dummy.Name.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_POSITION);
		aChild.SetContents(Format(buffer, Dummy.Position));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_DIRECTION);
		aChild.SetContents(Format(buffer, Dummy.Direction));

		aElement.AppendText("\n\t\t");
	}
	aDummyListElement.AppendText("\n\t");


	return true;
}

_NAMESPACE_REALSPACE2_END