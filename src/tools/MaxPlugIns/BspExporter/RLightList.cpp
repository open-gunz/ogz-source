#include "stdafx.h"
#include "MXml.h"
#include "RLightList.h"
#include "RToken.h"

_NAMESPACE_REALSPACE2_BEGIN

RLightList::~RLightList()
{
	for(iterator i=begin();i!=end();i++)
		delete *i;
}

bool RLightList::Open(MXmlElement *pElement)
{
	MXmlElement	aLightNode,aChild;
	int nCount = pElement->GetChildNodeCount();

	char szTagName[256],szContents[256];
	for (int i = 0; i < nCount; i++)
	{
		aLightNode = pElement->GetChildNode(i);
		aLightNode.GetTagName(szTagName);

		if(stricmp(szTagName,RTOK_LIGHT)==0)
		{
			RLIGHT *plight=new RLIGHT;
			aLightNode.GetAttribute(szContents,RTOK_NAME);
			plight->Name=szContents;
			plight->dwFlags=0;

			int nChildCount=aLightNode.GetChildNodeCount();
			for(int j=0;j<nChildCount;j++)
			{
				aChild = aLightNode.GetChildNode(j);
				aChild.GetTagName(szTagName);
				aChild.GetContents(szContents);

	#define READVECTOR(v) sscanf(szContents,"%f %f %f",&v.x,&v.y,&v.z)

				if(stricmp(szTagName,RTOK_POSITION)==0)		READVECTOR(plight->Position); else
				if(stricmp(szTagName,RTOK_COLOR)==0)		READVECTOR(plight->Color); else
				if(stricmp(szTagName,RTOK_INTENSITY)==0)	sscanf(szContents,"%f",&plight->fIntensity); else
				if(stricmp(szTagName,RTOK_ATTNSTART)==0)	sscanf(szContents,"%f",&plight->fAttnStart); else
				if(stricmp(szTagName,RTOK_ATTNEND)==0)		sscanf(szContents,"%f",&plight->fAttnEnd); else
				if(stricmp(szTagName,RTOK_CASTSHADOW)==0)	plight->dwFlags|=RM_FLAG_CASTSHADOW;
			}

			push_back(plight);
		}
	}
	return true;
}

bool RLightList::Save(MXmlElement *pElement)
{
	MXmlElement	aLightListElement=pElement->CreateChildElement(RTOK_LIGHTLIST);

	for(RLightList::iterator i=begin();i!=end();i++)
	{
		aLightListElement.AppendText("\n\t\t");

		RLIGHT *plight=*i;
		char buffer[256];

		MXmlElement		aElement,aChild;
		aElement = aLightListElement.CreateChildElement(RTOK_LIGHT);

		aElement.AddAttribute(RTOK_NAME,plight->Name.c_str());

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_POSITION);
		aChild.SetContents(Format(buffer,plight->Position));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_COLOR);
		aChild.SetContents(Format(buffer,plight->Color));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_INTENSITY);
		aChild.SetContents(Format(buffer,plight->fIntensity));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNSTART);
		aChild.SetContents(Format(buffer,plight->fAttnStart));

		aElement.AppendText("\n\t\t\t");
		aChild=aElement.CreateChildElement(RTOK_ATTNEND);
		aChild.SetContents(Format(buffer,plight->fAttnEnd));

		{
			MXmlElement aFlagElement;

			if((plight->dwFlags & RM_FLAG_CASTSHADOW) !=0)
			{
				aElement.AppendText("\n\t\t\t");
				aElement.CreateChildElement(RTOK_CASTSHADOW);
			}
		}
		aElement.AppendText("\n\t\t");
	}
	aLightListElement.AppendText("\n\t");
	return true;
}

_NAMESPACE_REALSPACE2_END
