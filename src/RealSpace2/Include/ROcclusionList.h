#pragma	once

#include <string>
#include "RTypes.h"
#include "RNameSpace.h"
#include "rapidxml.hpp"

_NAMESPACE_REALSPACE2_BEGIN

class ROcclusion {
public:
	void CalcPlane();

	std::vector<v3> Vertices;
	std::unique_ptr<rplane[]> Planes;
	rplane plane;
	std::string Name;
};

class ROcclusionList final : public std::vector<ROcclusion> {
public:
	bool Open(rapidxml::xml_node<>& parent);
	bool Save(MXmlElement *pElement);

	void UpdateCamera(const rmatrix &matWorld, const rvector &cameraPos);
	bool IsVisible(const rboundingbox &bb) const;

};

_NAMESPACE_REALSPACE2_END
