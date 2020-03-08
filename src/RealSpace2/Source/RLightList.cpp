#include "stdafx.h"
#include "MXml.h"
#include "RLightList.h"
#include "RToken.h"
#include "MXml.h"

_NAMESPACE_REALSPACE2_BEGIN

bool RLightList::Open(rapidxml::xml_node<>& parent)
{
	for (auto* node = parent.first_node(); node; node = node->next_sibling())
	{
		auto* szTagName = node->name();
		if (!szTagName)
			continue;

		if (_stricmp(szTagName, RTOK_LIGHT) != 0)
			continue;

		auto name_attr = node->first_attribute(RTOK_NAME);
		if (!name_attr)
			continue;

		auto* mat_name = name_attr->value();
		if (!mat_name)
			continue;

		emplace_back();
		RLIGHT& Light = back();
		Light.dwFlags = 0;
		Light.Name = mat_name;

		for (auto* prop_node = node->first_node(); prop_node; prop_node = prop_node->next_sibling())
		{
			auto* szTagName = prop_node->name();
			if (!szTagName)
				continue;
			const auto* szContents = prop_node->value();
			if (!szContents)
				szContents = "";

			auto ReadVector = [&](auto& v) {
				sscanf(szContents, "%f %f %f", &v.x, &v.y, &v.z);
			};

			auto ReadFloat = [&](auto& f) {
				f = float(atof(szContents));
			};

			if (_stricmp(szTagName, RTOK_POSITION) == 0)
				ReadVector(Light.Position);
			else if (_stricmp(szTagName, RTOK_COLOR) == 0)
				ReadVector(Light.Color);
			else if (_stricmp(szTagName, RTOK_INTENSITY) == 0)
				ReadFloat(Light.fIntensity);
			else if (_stricmp(szTagName, RTOK_ATTNSTART) == 0)
				ReadFloat(Light.fAttnStart);
			else if (_stricmp(szTagName, RTOK_ATTNEND) == 0)
				ReadFloat(Light.fAttnEnd);
			else if (_stricmp(szTagName, RTOK_CASTSHADOW) == 0)
				Light.dwFlags |= RM_FLAG_CASTSHADOW;
		}
	}
	return true;
}

bool RLightList::Save(MXmlElement *pElement)
{
	MXmlElement	aLightListElement=pElement->CreateChildElement(RTOK_LIGHTLIST);

	for(auto& Light : *this)
	{
		aLightListElement.AppendText("\n\t\t");

		char buffer[256];

		MXmlElement		aElement,aChild;
		aElement = aLightListElement.CreateChildElement(RTOK_LIGHT);

		aElement.AddAttribute(RTOK_NAME,Light.Name.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_POSITION);
		aChild.SetContents(Format(buffer,Light.Position));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_COLOR);
		aChild.SetContents(Format(buffer,Light.Color));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_INTENSITY);
		aChild.SetContents(Format(buffer,Light.fIntensity));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNSTART);
		aChild.SetContents(Format(buffer,Light.fAttnStart));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNEND);
		aChild.SetContents(Format(buffer,Light.fAttnEnd));

		{
			MXmlElement aFlagElement;

			if((Light.dwFlags & RM_FLAG_CASTSHADOW) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_CASTSHADOW);
			}
		}
		aElement.AppendText("\n\t\t");
	}
	aLightListElement.AppendText("\n\t");
	return true;
}

_NAMESPACE_REALSPACE2_END
