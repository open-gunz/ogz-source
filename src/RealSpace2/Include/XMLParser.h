/*
* Class to load RS3 xml's
*/
#pragma once

#include <vector>
#include "XMLFileStructs.h"

namespace rsx {

typedef std::vector<XMLMaterial> XMLMaterialVector;
typedef std::vector<XMLObject> XMLObjectVector;
typedef std::vector<XMLLight> XMLLightVector;

namespace XMLParser
{
/*
* Parses .elu.xml files
* @param FileName xml file name
* @param Ret reference to the XMLMaterialVector.
*/
bool parseXMLMaterial(const char *FileName, XMLMaterialVector &Ret);

bool parseScene(const char *FileName, XMLActor &actor, XMLObjectVector &Ret, XMLLightVector *Ret2 = nullptr);

bool parseProp(const char *FileName, XMLObjectVector &Ret);
};

}