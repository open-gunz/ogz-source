#ifndef _MXMLPARSER_H
#define _MXMLPARSER_H


class MXmlElement;
class MZFileSystem;

class MXmlParser
{
protected:
	virtual void ParseRoot(const char* szTagName, MXmlElement* pElement) = 0;
public:
	MXmlParser() {}
	virtual ~MXmlParser() {}
	bool ReadXml(const char* szFileName);								///< xml로부터 정보를 읽는다.
	bool ReadXml(MZFileSystem* pFileSystem,const char* szFileName);		///< xml로부터 정보를 읽는다.
};



#endif