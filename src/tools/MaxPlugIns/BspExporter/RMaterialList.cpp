#include "stdafx.h"
//#include <sys/stat.h>
#include "MXml.h"
#include "MDebug.h"
#include "RMaterialList.h"

_NAMESPACE_REALSPACE2_BEGIN

RMaterialList::~RMaterialList()
{
	for(iterator i=begin();i!=end();i++)
		delete *i;
}

/*
bool RMaterialList::Save(const char *szFileName)
{
	FILE *file=fopen(szFileName,"w");
	if(!file) return false;	
	for(iterator i=begin();i!=end();i++)
	{
		RMATERIAL *pMaterial=*i;
		if(!pMaterial->Name.empty())		// 이름이 있어야 한다.
		{
			fprintf(file,"%s { \n",RTOK_HEADER);
			fprintf(file,"%s \"%s\"\n",RTOK_NAME,pMaterial->Name.c_str());
			fprintf(file,"%s %x\n",RTOK_AMBIENT,pMaterial->Ambient);
			fprintf(file,"%s %x\n",RTOK_DIFFUSE,pMaterial->Diffuse);
			fprintf(file,"%s %x\n",RTOK_SPECULAR,pMaterial->Specular);
			fprintf(file,"%s %f\n",RTOK_POWER,pMaterial->Power);
			if(!pMaterial->DiffuseMap.empty())
				fprintf(file,"%s \"%s\"\n",RTOK_DIFFUSEMAP,pMaterial->DiffuseMap.c_str());
			
			// 플래그들.
			if((pMaterial->dwFlags & RM_FLAG_ADDITIVE) !=0 )
				fprintf(file,"%s\n",RTOK_ADDITIVE);
			if((pMaterial->dwFlags & RM_FLAG_USEOPACITY) !=0 )
				fprintf(file,"%s\n",RTOK_USEOPACITY);
			if((pMaterial->dwFlags & RM_FLAG_TWOSIDED) !=0 )
				fprintf(file,"%s\n",RTOK_TWOSIDED);

			fprintf(file,"} \n");
		}
	}
	fclose(file);

	return true;
}

bool RMaterialList::Open(void *pMemory,int nSize)
{
	char *pCurrent=(char*)pMemory;
	list<string> words;

	while(pCurrent<(char*)pMemory+nSize)
	{
		int nPos=
			(*pCurrent=='{' || *pCurrent=='}') ? 1 : 
			*pCurrent=='\"' ? strcspn(pCurrent+1,"\"")+2 : strcspn( pCurrent, "{}\t\n\r " );
		if(nPos>0)
		{
			words.push_back(string(pCurrent,pCurrent+nPos));
			pCurrent+=nPos;
		} else pCurrent++;
	}

	list<string>::iterator i;

	i=words.begin();
	while(i!=words.end())
	{
		if(*i==RTOK_HEADER)
		{
			if(*++i=="{")
			{
				RMATERIAL *pMaterial=new RMATERIAL;
				pMaterial->dwFlags=0;
				while(i!=words.end() && *i!="}")
				{
					i++;
					if(*i==RTOK_NAME) {i++;pMaterial->Name=(*i).substr(1,(*i).length()-2);} else
					if(*i==RTOK_DIFFUSEMAP) {i++;pMaterial->DiffuseMap=(*i).substr(1,(*i).length()-2);} else
					if(*i==RTOK_AMBIENT) sscanf((*++i).c_str(),"%x",&pMaterial->Ambient); else
					if(*i==RTOK_DIFFUSE) sscanf((*++i).c_str(),"%x",&pMaterial->Diffuse); else
					if(*i==RTOK_SPECULAR) sscanf((*++i).c_str(),"%x",&pMaterial->Specular); else
					if(*i==RTOK_POWER) sscanf((*++i).c_str(),"%f",&pMaterial->Power); else
					if(*i==RTOK_ADDITIVE) {pMaterial->dwFlags|=RM_FLAG_ADDITIVE;} else
					if(*i==RTOK_USEOPACITY) {pMaterial->dwFlags|=RM_FLAG_USEOPACITY;} else
					if(*i==RTOK_TWOSIDED) {pMaterial->dwFlags|=RM_FLAG_TWOSIDED;}
				}
				if(*i=="}")
				{
					push_back(pMaterial);
					i++;
				}
				else mlog("} needed.\n");
			}
			else
			{
				mlog("{ needed.\n");
				i++;
			}
		}
		else
		{
			mlog("%s needed.\n",RTOK_HEADER);
			i++;
		}
	}
	return true;
}

bool RMaterialList::Open(const char *szFileName)
{
	struct _stat buf;
	if(_stat(szFileName,&buf)!=0)
		return false;

	FILE *file=fopen(szFileName,"rb");
	char *buffer=new char[buf.st_size];
	fread(buffer,buf.st_size,1,file);
	fclose(file);
	bool bReturn=Open((void*)buffer,buf.st_size);
	delete []buffer;
	return bReturn;
}
*/

bool RMaterialList::Open(MXmlElement *pElement)
{
	MXmlElement	aMaterialNode,aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256],szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aMaterialNode = pElement->GetChildNode(i);
		aMaterialNode.GetTagName(szTagName);

		if(stricmp(szTagName,RTOK_MATERIAL)==0)
		{
			RMATERIAL *pMaterial=new RMATERIAL;
			pMaterial->dwFlags=0;
			aMaterialNode.GetAttribute(szContents,RTOK_NAME);
			pMaterial->Name=szContents;

			int nChildCount=aMaterialNode.GetChildNodeCount();
			for(int j=0;j<nChildCount;j++)
			{
				aChild = aMaterialNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

#define READVECTOR(v) sscanf(szContents,"%f %f %f",&v.x,&v.y,&v.z)

				if(stricmp(szTagName,RTOK_AMBIENT)==0)		READVECTOR(pMaterial->Ambient); else
				if(stricmp(szTagName,RTOK_DIFFUSE)==0)		READVECTOR(pMaterial->Diffuse); else
				if(stricmp(szTagName,RTOK_SPECULAR)==0)		READVECTOR(pMaterial->Specular); else
				if(stricmp(szTagName,RTOK_DIFFUSEMAP)==0)	pMaterial->DiffuseMap=szContents; else
				if(stricmp(szTagName,RTOK_POWER)==0)		sscanf(szContents,"%f",&pMaterial->Power); else
				if(stricmp(szTagName,RTOK_ADDITIVE)==0)		pMaterial->dwFlags|=RM_FLAG_ADDITIVE; else
				if(stricmp(szTagName,RTOK_USEOPACITY)==0)	pMaterial->dwFlags|=RM_FLAG_USEOPACITY; else
				if(stricmp(szTagName,RTOK_TWOSIDED)==0)		pMaterial->dwFlags|=RM_FLAG_TWOSIDED; else
				if(stricmp(szTagName,RTOK_USEALPHATEST)==0)	pMaterial->dwFlags|=RM_FLAG_USEALPHATEST;
			}

			push_back(pMaterial);
		}
	}
	return true;
}

bool RMaterialList::Save(MXmlElement *pElement)
{
	MXmlElement	aMaterialListElement = pElement->CreateChildElement(RTOK_MATERIALLIST);

	{
		for(iterator i=begin();i!=end();i++)
		{
			aMaterialListElement.AppendText("\n\t\t");

			RMATERIAL *pMaterial=*i;

			char buffer[256];

			MXmlElement		aElement,aChild;
			aElement=aMaterialListElement.CreateChildElement(RTOK_MATERIAL);
			aElement.AddAttribute(RTOK_NAME,pMaterial->Name.c_str());

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_DIFFUSE);
			aChild.SetContents(Format(buffer,pMaterial->Diffuse));

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_AMBIENT);
			aChild.SetContents(Format(buffer,pMaterial->Ambient));

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_SPECULAR);
			aChild.SetContents(Format(buffer,pMaterial->Specular));

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_DIFFUSEMAP);
			aChild.SetContents(pMaterial->DiffuseMap.c_str());

			{
				MXmlElement aFlagElement;

				if((pMaterial->dwFlags & RM_FLAG_ADDITIVE) !=0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_ADDITIVE);
				}
				if((pMaterial->dwFlags & RM_FLAG_TWOSIDED) !=0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_TWOSIDED);
				}
				if((pMaterial->dwFlags & RM_FLAG_USEOPACITY) !=0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_USEOPACITY);
				}
				if((pMaterial->dwFlags & RM_FLAG_USEALPHATEST) !=0)
				{
					aElement.AppendText("\n\t\t\t");
					aElement.CreateChildElement(RTOK_USEALPHATEST);
				}
			}
			aElement.AppendText("\n\t\t");
		}
		aMaterialListElement.AppendText("\n\t");
	}
	return true;
}

_NAMESPACE_REALSPACE2_END
