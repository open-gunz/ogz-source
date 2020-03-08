#include <string.h>
#include "BEParser.h"
#include "RSScanner.h"

char *reservedword[]={ "destination",	"export",	"animation",	"transform",	"vertex",	"keyframe",
						"source",		"rmlfile"};
enum reservedcode	{ ISTART=0,
						IDESTINATION,	IEXPORT,	IANIMATION,		ITRANSFORM,		IVERTEX,	IKEYFRAME,
						ISOURCE,		IRMLFILE,
						IUNDEFINED	};
char errormessage[256];

BEParser::BEParser()
{
	pScanner=NULL;
}

BEParser::~BEParser()
{
	if(pScanner)
	{
		delete pScanner;
		pScanner=NULL;
	}
}

reservedcode BEParser::GetReserved(const char *szString)
{
	for(int i=0;i<IUNDEFINED-1;i++)
		if(stricmp(szString,reservedword[i])==0)
		{
			return (reservedcode)(i+1);
		}
	return IUNDEFINED;
}

bool BEParser::Parse_Destination()
{
	pScanner->ReadToken();
	RTOKENTYPE tt=pScanner->GetTokenType();
	if((tt!=RTOKEN_STRING)&&(tt!=RTOKEN_CONSTANTSTRING)) return false;
	
	BECommand *pCommand=new BECommand;
	pScanner->GetToken(pCommand->szBuffer,sizeof(pCommand->szBuffer));
	pCommand->nCommand=CDESTINATION;
	CommandList.Add(pCommand);

	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	if(tt!=RTOKEN_SEMICOLON) return false;
	return true;
}

bool BEParser::Parse_Source()
{
	pScanner->ReadToken();
	RTOKENTYPE tt=pScanner->GetTokenType();
	if((tt!=RTOKEN_STRING)&&(tt!=RTOKEN_CONSTANTSTRING)) return false;
	
	BECommand *pCommand=new BECommand;
	pScanner->GetToken(pCommand->szBuffer,sizeof(pCommand->szBuffer));
	pCommand->nCommand=CSOURCE;
	CommandList.Add(pCommand);

	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	if(tt!=RTOKEN_SEMICOLON) return false;
	return true;
}

bool BEParser::Parse_RmlFile()
{
	pScanner->ReadToken();
	RTOKENTYPE tt=pScanner->GetTokenType();
	if((tt!=RTOKEN_STRING)&&(tt!=RTOKEN_CONSTANTSTRING)) return false;
	
	BECommand *pCommand=new BECommand;
	pScanner->GetToken(pCommand->szBuffer,sizeof(pCommand->szBuffer));
	pCommand->nCommand=CRMLFILE;
	CommandList.Add(pCommand);

	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	if(tt!=RTOKEN_SEMICOLON) return false;
	return true;
}

bool BEParser::Parse_Export_Animation(BEAnimationList *pAnimationList)
{
	pScanner->ReadToken();
	RTOKENTYPE tt=pScanner->GetTokenType();
	if(tt==RTOKEN_SEMICOLON) { return true; }
	if(tt!=RTOKEN_STRING) { return false; }
	if(GetReserved(pScanner->GetToken())!=IANIMATION) return false;

	pScanner->ReadToken();
	AnimationParam *newani=new AnimationParam;
	reservedcode rc=GetReserved(pScanner->GetToken());
	switch(rc)
	{
		case ITRANSFORM	: newani->iAnimationType=AM_TRANSFORM;break;
		case IVERTEX	: newani->iAnimationType=AM_VERTEX;break;
		case IKEYFRAME	: newani->iAnimationType=AM_KEYFRAME;break;
		default : delete newani;return false;
	}
		
	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	if((tt!=RTOKEN_STRING)&&(tt!=RTOKEN_CONSTANTSTRING)) return false;
	pScanner->GetToken(newani->szAnimationName,sizeof(newani->szAnimationName));
	
	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	switch(tt)
	{
		case RTOKEN_NUMBER : {int i;pScanner->GetToken(&i);newani->fAnimationSpeed=(float)i;}break;
		case RTOKEN_REALNUMBER : { float f;pScanner->GetToken(&f);newani->fAnimationSpeed=f;}break;
		default : delete newani;return false;
	}

	pScanner->ReadToken();
	tt=pScanner->GetTokenType();
	if((tt!=RTOKEN_STRING)&&(tt!=RTOKEN_CONSTANTSTRING)) return false;
	pScanner->GetToken(newani->szMaxFileName,sizeof(newani->szMaxFileName));

	pAnimationList->Add(newani);
	return Parse_Export_Animation(pAnimationList);
}

bool BEParser::Parse_Export()
{
	BECommand *pCommand=new BECommand;
	pCommand->nCommand=CEXPORT;
	pScanner->ReadToken();
	RTOKENTYPE tt=pScanner->GetTokenType();
	if((tt==RTOKEN_STRING)||(tt==RTOKEN_CONSTANTSTRING))
	{
		pScanner->GetToken(pCommand->szBuffer,sizeof(pCommand->szBuffer));
		pScanner->ReadToken();
		tt=pScanner->GetTokenType();
		if((tt==RTOKEN_STRING)||(tt==RTOKEN_CONSTANTSTRING))
		{
			pScanner->GetToken(pCommand->szBuffer2,sizeof(pCommand->szBuffer2));
			CommandList.Add(pCommand);
			return Parse_Export_Animation(&pCommand->AnimationList);
		}
	}	
	delete pCommand;
	return false;
}

bool BEParser::Open(const char *filename)
{
	FILE *file;
	file=fopen(filename,"r");
	if(!file) return false;

	pScanner=new RSScanner;
	pScanner->Open(file);

	while(pScanner->GetTokenType()!=RTOKEN_NULL)
	{
		reservedcode rc=GetReserved(pScanner->GetToken());

		bool bReturn=0;
		switch ( rc )
		{
		case IRMLFILE		: bReturn=Parse_RmlFile();break;
		case ISOURCE		: bReturn=Parse_Source();break;
		case IDESTINATION	: bReturn=Parse_Destination();break;
		case IEXPORT		: bReturn=Parse_Export();break;
		default: goto ERROR_OCCURED;
		}
		if(!bReturn) goto ERROR_OCCURED;
		pScanner->ReadToken();
	}

	fclose(file);
	return true;

ERROR_OCCURED:
	sprintf(errormessage,"parse error line number %d",pScanner->GetCurrentLineNumber());
	delete pScanner;
	pScanner=NULL;
	fclose(file);
	return false;
}

BECommandList *BEParser::GetCommandList()
{
	return &CommandList;
}

char *BEParser::GetErrorString()
{
	return errormessage;
}