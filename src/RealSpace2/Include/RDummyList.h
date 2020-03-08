#pragma once

#include <list>
#include <string>
#include "RTypes.h"
#include "RToken.h"
#include "rapidxml.hpp"

class MXmlElement;

#include "RNameSpace.h"
_NAMESPACE_REALSPACE2_BEGIN

struct RDummy {
	std::string Name;
	v3 Position;
	v3 Direction;
};

class RDummyList : public std::vector<RDummy>
{
public:
	bool Open(rapidxml::xml_node<>& parent);
	bool Save(MXmlElement *pElement);
};

_NAMESPACE_REALSPACE2_END
