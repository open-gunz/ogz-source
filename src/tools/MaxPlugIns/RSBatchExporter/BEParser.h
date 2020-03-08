#ifndef __BEPARSER_H
#define __BEPARSER_H

#include "CMList.h"
#include "AnimConstructor.h"

enum ICOMMAND { CDESTINATION,	CSOURCE,	CEXPORT,	CRMLFILE};

struct AnimationParam
{
	ANIMATIONMETHOD iAnimationType;
	float fAnimationSpeed;
	char szAnimationName[256];
	char szMaxFileName[256];
};
typedef CMLinkedList <AnimationParam> BEAnimationList;

struct BECommand
{
	ICOMMAND		nCommand;
	char			szBuffer[256],szBuffer2[256];
	BEAnimationList	AnimationList;
};
typedef CMLinkedList <BECommand>	BECommandList;

enum reservedcode;
class RSScanner;

class BEParser  
{
public:
	BEParser();
	virtual ~BEParser();

	bool Open(const char *filename);
	BECommandList *GetCommandList();
	char *GetErrorString();

private:

	BECommandList CommandList;
	RSScanner *pScanner;

	reservedcode GetReserved(const char *szString);
	bool Parse_RmlFile();
	bool Parse_Destination();
	bool Parse_Source();
	bool Parse_Export_Animation(BEAnimationList *pAnimationList);
	bool Parse_Export();
};

#endif
