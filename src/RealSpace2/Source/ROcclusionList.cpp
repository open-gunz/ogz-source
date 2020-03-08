#include "stdafx.h"
#include "MXml.h"
#include "ROcclusionList.h"
#include "RToken.h"
#include "RealSpace2.h"

_NAMESPACE_REALSPACE2_BEGIN

bool ROcclusionList::Open(rapidxml::xml_node<>& parent)
{
	for (auto* node = parent.first_node(); node; node = node->next_sibling())
	{
		auto* szTagName = node->name();
		if (!szTagName)
			continue;

		if (_stricmp(szTagName, RTOK_OCCLUSION) != 0)
			continue;

		auto name_attr = node->first_attribute(RTOK_NAME);
		if (!name_attr)
			continue;

		auto* mat_name = name_attr->value();
		if (!mat_name)
			continue;

		emplace_back();
		auto& Occlusion = back();
		Occlusion.Name = mat_name;

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

			if (_stricmp(szTagName, RTOK_POSITION) == 0) {
				v3 temp;
				ReadVector(temp);
				Occlusion.Vertices.push_back(temp);
			}
		}

		Occlusion.Planes = std::unique_ptr<rplane[]>{ new rplane[Occlusion.Vertices.size() + 1] };
		Occlusion.CalcPlane();
	}
	return true;
}

bool ROcclusionList::Save(MXmlElement *pElement)
{
	MXmlElement	aOcclusionListElement=pElement->CreateChildElement(RTOK_OCCLUSIONLIST);

	for(auto& Occlusion : *this)
	{
		aOcclusionListElement.AppendText("\n\t\t");

		char buffer[256];

		MXmlElement aElement,aChild;
		aElement = aOcclusionListElement.CreateChildElement(RTOK_OCCLUSION);

		aElement.AddAttribute(RTOK_NAME, Occlusion.Name.c_str());

		for (size_t j = 0; j < Occlusion.Vertices.size(); j++)
		{
			aElement.AppendText("\n\t\t\t");

			aChild=aElement.CreateChildElement(RTOK_POSITION);
			aChild.SetContents(Format(buffer, Occlusion.Vertices[j]));
		}

		aElement.AppendText("\n\t\t");
	}
	aOcclusionListElement.AppendText("\n\t");
	return true;
}

bool ROcclusionList::IsVisible(const rboundingbox &bb) const
{
	for(auto& Occlusion : *this)
	{
		bool bVisible = false;

		for (size_t j = 0; j < Occlusion.Vertices.size() + 1; j++)
		{
			if (isInPlane(bb, Occlusion.Planes[j]))
			{
				bVisible = true;
				break;
			}
		}

		if (!bVisible)
			return false;
	}

	return true;
}

void ROcclusionList::UpdateCamera(const rmatrix &matWorld, const rvector &cameraPos)
{
	rmatrix invWorld = Inverse(matWorld);
	rvector localCameraPos = cameraPos * invWorld;
	// We need the inverse transpose for proper plane transformation.
	rmatrix trInvWorld = Transpose(invWorld);

	for (auto& Occlusion : *this)
	{
		bool bm_pPositive = DotProduct(Occlusion.plane, localCameraPos) > 0;

		Occlusion.Planes[0] *= trInvWorld;

		Occlusion.Planes[0] = bm_pPositive ? Occlusion.plane : -Occlusion.plane;
		for (size_t j = 0; j < Occlusion.Vertices.size(); j++)
		{
			auto& Plane = Occlusion.Planes[j + 1];
			if (bm_pPositive)
				Occlusion.Planes[j + 1] = PlaneFromPoints(
					Occlusion.Vertices[j],
					Occlusion.Vertices[(j + 1) % Occlusion.Vertices.size()],
					localCameraPos);
			else
				Occlusion.Planes[j + 1] = PlaneFromPoints(
					Occlusion.Vertices[(j + 1) % Occlusion.Vertices.size()],
					Occlusion.Vertices[j],
					localCameraPos);

			Plane *= trInvWorld;
		}
	}
}

_NAMESPACE_REALSPACE2_END

void ROcclusion::CalcPlane()
{
	plane = PlaneFromPoints(Vertices[0], Vertices[1], Vertices[2]);
}