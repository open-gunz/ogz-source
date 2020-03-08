#pragma once

#include <list>
#include <string>
#include "RTypes.h"
#include "RToken.h"
#include "RNameSpace.h"
#include "rapidxml.hpp"

class MXmlElement;

_NAMESPACE_REALSPACE2_BEGIN

struct RMATERIAL {
	std::string Name;
	v3 Diffuse;
	v3 Ambient;
	v3 Specular;
	float Power;
	std::string DiffuseMap;
	u32 dwFlags;
};

class RMaterialList : public std::vector<RMATERIAL> {
public:
	bool Open(rapidxml::xml_node<>&);
	bool Save(MXmlElement*);
};

_NAMESPACE_REALSPACE2_END