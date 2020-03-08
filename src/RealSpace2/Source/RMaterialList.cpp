#include "stdafx.h"
#include "MXml.h"
#include "MDebug.h"
#include "RMaterialList.h"

_NAMESPACE_REALSPACE2_BEGIN

bool RMaterialList::Open(rapidxml::xml_node<>& parent)
{
	for (auto* node = parent.first_node(); node; node = node->next_sibling())
	{
		auto* szTagName = node->name();
		if (!szTagName)
			continue;

		if (_stricmp(szTagName, RTOK_MATERIAL) != 0)
			continue;

		auto name_attr = node->first_attribute(RTOK_NAME);
		if (!name_attr)
			continue;

		auto* mat_name = name_attr->value();
		if (!mat_name)
			continue;

		emplace_back();
		RMATERIAL& Material = back();
		Material.dwFlags = 0;
		Material.Name = mat_name;

		for (auto* prop_node = node->first_node(); prop_node; prop_node = prop_node->next_sibling())
		{
			auto* szTagName = prop_node->name();
			if (!szTagName)
				continue;
			const char* szContents = prop_node->value();
			if (!szContents)
				szContents = "";

			auto ReadVector = [&](auto& v) {
				sscanf(szContents, "%f %f %f", &v.x, &v.y, &v.z);
			};

			if (_stricmp(szTagName, RTOK_AMBIENT) == 0)
				ReadVector(Material.Ambient);
			else if (_stricmp(szTagName, RTOK_DIFFUSE) == 0)
				ReadVector(Material.Diffuse);
			else if (_stricmp(szTagName, RTOK_SPECULAR) == 0)
				ReadVector(Material.Specular);
			else if (_stricmp(szTagName, RTOK_DIFFUSEMAP) == 0)
				Material.DiffuseMap = szContents;
			else if (_stricmp(szTagName, RTOK_POWER) == 0)
				Material.Power = float(atof(szContents));
			else if (_stricmp(szTagName, RTOK_ADDITIVE) == 0)
				Material.dwFlags |= RM_FLAG_ADDITIVE;
			else if (_stricmp(szTagName, RTOK_USEOPACITY) == 0)
				Material.dwFlags |= RM_FLAG_USEOPACITY;
			else if (_stricmp(szTagName, RTOK_TWOSIDED) == 0)
				Material.dwFlags |= RM_FLAG_TWOSIDED;
			else if (_stricmp(szTagName, RTOK_USEALPHATEST) == 0)
				Material.dwFlags |= RM_FLAG_USEALPHATEST;
		}
	}

	return true;
}

bool RMaterialList::Save(MXmlElement* pElement)
{
	MXmlElement	aMaterialListElement = pElement->CreateChildElement(RTOK_MATERIALLIST);

	for (auto& Material : *this)
	{
		aMaterialListElement.AppendText("\n\t\t");

		char buffer[256];

		MXmlElement		aElement,aChild;
		aElement=aMaterialListElement.CreateChildElement(RTOK_MATERIAL);
		aElement.AddAttribute(RTOK_NAME,Material.Name.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_DIFFUSE);
		aChild.SetContents(Format(buffer,Material.Diffuse));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_AMBIENT);
		aChild.SetContents(Format(buffer,Material.Ambient));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_SPECULAR);
		aChild.SetContents(Format(buffer,Material.Specular));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_DIFFUSEMAP);
		aChild.SetContents(Material.DiffuseMap.c_str());

		{
			MXmlElement aFlagElement;

			if((Material.dwFlags & RM_FLAG_ADDITIVE) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_ADDITIVE);
			}
			if((Material.dwFlags & RM_FLAG_TWOSIDED) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_TWOSIDED);
			}
			if((Material.dwFlags & RM_FLAG_USEOPACITY) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_USEOPACITY);
			}
			if((Material.dwFlags & RM_FLAG_USEALPHATEST) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_USEALPHATEST);
			}
		}
		aElement.AppendText("\n\t\t");
	}
	aMaterialListElement.AppendText("\n\t");

	return true;
}

_NAMESPACE_REALSPACE2_END
