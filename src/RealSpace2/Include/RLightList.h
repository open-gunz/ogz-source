#pragma once

#include <string>
#include <vector>
#include "RTypes.h"
#include "RNameSpace.h"
#include "rapidxml.hpp"

class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RLIGHT
{
	std::string Name;
	v3 Color;
	v3 Position;
	float fIntensity;
	float fAttnStart, fAttnEnd;
	u32	dwFlags;
};

struct RLightList : public std::vector<RLIGHT> {
	bool Open(rapidxml::xml_node<>& parent);
	bool Save(MXmlElement *pElement);
};

_NAMESPACE_REALSPACE2_END
