// RSBspExporter.cpp: implementation of the RSBspExporter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include "MXml.h"

#include "RSBspExporter.h"
#include "RMaterialList.h"
#include "RVersions.h"

#include "FileInfo.h"
#include "BELog.h"
#include "DialogProc.h"
#include "RToken.h"
#include "Max.h"

#include "MDebug.h"

//#include <d3dx8.h>

// 이 정도 개수 이하는 쪼개지 않는다 ( octree )
#define MAX_NODE_POLYGON_COUNT		200

RSBspExporter::RSBspExporter()
{
	nMesh=0;
//	meshhead=NULL;mesh=NULL;
	BspHead=NULL;
	OcHead=NULL;

	MapName[0] = NULL;

//	m_pNavBspRoot = NULL;
}
 
RSBspExporter::~RSBspExporter()
{
	/*
	RSMMesh *m=meshhead,*next;
	while(m)
	{
		next=m->next;
		delete m;
		m=next;
	}
	*/
	if(BspHead) delete BspHead;
	if(OcHead) delete OcHead;
}

void RSBspExporter::Preprocessing()
{
	/*
	// prework for save... 
	mesh=meshhead;
	RSMMesh **prevmesh=&meshhead;
	while(mesh)
	{
		log("Mesh %s",mesh->name);
		char header[4];					// for filtering Bipxxxx named object from character studio.
		header[3]=0;
		memcpy(header,mesh->name,3);
		if( (mesh->nF==0) ||			// zero vertex or zero face mesh
			(strcmp(header,"Bip")==0)) // it will be deleted. cause it is biped
		{
			log(" -> Deleted. \n");
			nMesh--;
			RSMMesh *todelete=mesh;
			mesh=mesh->next;
			delete todelete;
			*prevmesh=mesh;
		}
		else
		{
			log("\n");

			prevmesh=&mesh->next;
			mesh=mesh->next;
		}
	}
	log("Preprocessing ok.\n");



	// vertex 카피가 들어갑니다 ~
	dpoint light=dpoint(1,0,1);
	Normalize(light);

	int i,j,elimcount=0;

	RSMMesh *mesh=meshhead;
	for(i=0;i<nMesh;i++)
	{
#define EQ(a,b) (IS_EQ(a.x,b.x)&&IS_EQ(a.y,b.y)&&IS_EQ(a.z,b.z))
		for(j=0;j<mesh->nF;j++)
		{
			rpolygon *f=new rpolygon;
			memcpy(f,&mesh->face[j],sizeof(rpolygon));			
			f->v=new rvertex[mesh->face[j].nCount];
			memcpy(f->v,mesh->face[j].v,f->nCount*sizeof(rvertex));
			f->normal=mesh->face[j].normal;
			f->d=mesh->face[j].d;
			f->nMaterial=mesh->face[j].nMaterial;
			f->dwFlags=mesh->face[j].dwFlags;

			if(EQ(f->v[0].coord,f->v[1].coord)||
				EQ(f->v[1].coord,f->v[2].coord)||
				EQ(f->v[2].coord,f->v[0].coord))
			{
				delete f;
				elimcount++;
			}				
			else
				face.Add(f);
		}

		mesh=mesh->next;
	}
	*/

	log("RSBsp Info : %d faces\n",face.GetCount());
//	log("%d faces elliminated.\n",elimcount);

	// 그리고 바운딩박스.
	m_bb.vmin.x=m_bb.vmin.y=m_bb.vmin.z=FLT_MAX;
	m_bb.vmax.x=m_bb.vmax.y=m_bb.vmax.z=-FLT_MAX;

	for(int i=0;i<face.GetCount();i++)
	{
		rpolygon *f=face.Get(i);
		for(int j=0;j<f->nCount;j++)
		{
			for(int k=0;k<3;k++)
			{
				m_bb.vmin[k]=min(m_bb.vmin[k],f->v[j].coord[k]);
				m_bb.vmax[k]=max(m_bb.vmax[k],f->v[j].coord[k]);
			}
		}
	}

	log("Scene Dimension ( %3.1f %3.1f %3.1f )\n",m_bb.maxx-m_bb.minx,m_bb.maxy-m_bb.miny,m_bb.maxz-m_bb.minz);
}

bool RSBspExporter::SaveRS(const char *name)
{
	FILE *file=fopen(name,"wb+");
	if(!file) return false;

	RHEADER header(RS_ID,RS_VERSION);
	fwrite(&header,sizeof(RHEADER),1,file);

	int nCount=MaxSubMaterialList.GetCount();
	fwrite(&nCount,sizeof(int),1,file);
	
	for(int i=0;i<nCount;i++)
	{
		const char *name=MaxSubMaterialList.Get(i)->Name.c_str();
		fwrite(name,strlen(name)+1,1,file);
	}

	log("Saving Source Data.\n");
	SaveSource(file);

	if(ip->GetCancel()) goto cancel;

	ConstructBspTree();

	if(ip->GetCancel()) goto cancel;

	log("Saving Bsp information.\n");

	bool ret=Save(file);
	if(!ret) {
		fclose(file);
		return false;
	}

	if(ip->GetCancel()) goto cancel;

	fclose(file);
	return ret;
cancel:
	fclose(file);

	remove(name);
	return false;
}

bool RSBspExporter::SaveDesc(const char *name)
{
	char MessageBuffer[4096]={0,};
	int nMaterial=MaxSubMaterialList.GetCount();
	RMaterialList *ml=new RMaterialList;
	for(int i=0;i<nMaterial;i++)
	{
		MaxSubMaterial *material=MaxSubMaterialList.Get(i);
		RMATERIAL *m=new RMATERIAL;

		m->Name=material->Name;
		m->Ambient=material->Ambient;
		m->Diffuse=material->Diffuse;
		m->Specular=material->Specular;
		m->dwFlags=material->dwFlags;
		if(material->DiffuseMap.length())
		{
			char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
			_splitpath(material->DiffuseMap.c_str(),drive,dir,fname,ext);
			m->DiffuseMap=string(fname)+string(ext);
		}

		ml->push_back(m);
	}

	MXmlDocument	aXml;
	
	aXml.Create();
	aXml.CreateProcessingInstruction();
	
	MXmlElement		aRootElement;

	aRootElement=aXml.CreateElement("XML");
	aRootElement.AppendText("\n\t");

	aXml.AppendChild(aRootElement);

	ml->Save(&aRootElement);

	aRootElement.AppendText("\n\t");

	m_StaticLightList.Save(&aRootElement);

	aRootElement.AppendText("\n\t");

	// save object list
	{
		MXmlElement	aObjectListElement=aRootElement.CreateChildElement(RTOK_OBJECTLIST);
		for(NODELIST::iterator i=m_ObjectsList.begin();i!=m_ObjectsList.end();i++)
		{
			INode *pNode=*i;

			aObjectListElement.AppendText("\n\t\t");

			MXmlElement		aElement,aChild;
			aElement=aObjectListElement.CreateChildElement(RTOK_OBJECT);

			char filename[256];

			sprintf(filename,"%s_%s.elu",MapName,pNode->GetName());

			aElement.AddAttribute(RTOK_NAME,filename);

			aElement.AppendText("\n\t\t");
		}
		aObjectListElement.AppendText("\n\t");
	}


	aRootElement.AppendText("\n\t");

	// save dummy list
	{
		MXmlElement	aListElement=aRootElement.CreateChildElement(RTOK_DUMMYLIST);

		for(list<RDUMMY*>::iterator i=m_DummyList.begin();i!=m_DummyList.end();i++)
		{
			aListElement.AppendText("\n\t\t");

			RDUMMY *psp=*i;
			char buffer[256];

			MXmlElement		aElement,aChild;
			aElement = aListElement.CreateChildElement(RTOK_DUMMY);

			aElement.AddAttribute(RTOK_NAME,psp->name.c_str());

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_POSITION);
			aChild.SetContents(Format(buffer,psp->position));

			aElement.AppendText("\n\t\t\t");
			aChild=aElement.CreateChildElement(RTOK_DIRECTION);
			aChild.SetContents(Format(buffer,psp->direction));

			aElement.AppendText("\n\t\t");
		}

		aListElement.AppendText("\n\t");
	}


	aRootElement.AppendText("\n\t");

	m_OcclusionList.Save(&aRootElement);

	SaveFog(aRootElement);
	SaveSoundProp(aRootElement);
	SaveCustomProperties(aRootElement);

	aRootElement.AppendText("\n");
	aXml.SaveToFile(name);


//	ml->Save(name);
	delete ml;

	if(MessageBuffer[0])
		MessageBox( NULL, MessageBuffer, "RSM Exporter Error Report", MB_OK );

	return true;
}

// spawn.xml에서 사용
#define RTOK_GAMETYPE		"GAMETYPE"
#define RTOK_SPAWN			"SPAWN"
#define RTOK_ITEM			"item"
#define RTOK_TIMESEC		"timesec"

bool RSBspExporter::SaveSpawn(const char* name)
{
	MXmlDocument	aXml;

	aXml.Create();
	aXml.CreateProcessingInstruction();

	MXmlElement		aRootElement;

	aRootElement=aXml.CreateElement("XML");
	aRootElement.AppendText("\n\t");
	aXml.AppendChild(aRootElement);


	for (int i = 0; i < 2; i++)
	{
		char szTok[256];
		if (i == 0) strcpy(szTok, "solo");
		else strcpy(szTok, "team");

		MXmlElement	aSoloGameTypeElement=aRootElement.CreateChildElement(RTOK_GAMETYPE);
		aSoloGameTypeElement.AddAttribute("id", szTok);

		for(list<RDUMMY*>::iterator i=m_DummyList.begin();i!=m_DummyList.end();i++)
		{
			RDUMMY *psp=*i;

			char szDummyName[256];
			sprintf(szDummyName, "spawn_item_%s", szTok);

			if (!strnicmp(psp->name.c_str(), szDummyName, strlen(szDummyName)))
			{
				if (strlen(psp->name.c_str()) < 16) continue;

				aSoloGameTypeElement.AppendText("\n\t\t");

				char buffer[256];

				MXmlElement aElement,aChild;

				aElement = aSoloGameTypeElement.CreateChildElement(RTOK_SPAWN);

				const char* pItemName = &psp->name.c_str()[16];
				char* plast = strchr(pItemName, '_');
				char szItemName[256];
				if (plast!=NULL)
				{
					memset(szItemName, 0, sizeof(szItemName));
					memcpy(szItemName, pItemName, plast-pItemName);
				}
				else
				{
					strcpy(szItemName, pItemName);
				}

				aElement.AddAttribute("item",szItemName);

				int nTimeSec = 0;
				if (!psp->GetUserPropValue("time", &nTimeSec))
				{
					nTimeSec = 0;
				}
				aElement.AddAttribute("timesec", nTimeSec*1000);

				aElement.AppendText("\n\t\t\t");
				aChild=aElement.CreateChildElement(RTOK_POSITION);
				aChild.SetContents(Format(buffer,psp->position));
				aElement.AppendText("\n\t\t");
			}
		}
		aSoloGameTypeElement.AppendText("\n");
	}

	aRootElement.AppendText("\n");
	aXml.SaveToFile(name);



//	if(MessageBuffer[0])
//		MessageBox( NULL, MessageBuffer, "RSM Exporter Error Report", MB_OK );

	return true;

}

bool RSBspExporter::SaveSmoke(const char* name)
{
	MXmlDocument	aXml;

	aXml.Create();
	aXml.CreateProcessingInstruction();

	MXmlElement	aRootElement;

	aRootElement=aXml.CreateElement("XML");
	aXml.AppendChild(aRootElement);

	const char* cszDummyName = "smk_";
	char strBuffer[256];
	int		iBuffer;
	float	fBuffer;

	int ntest = 0;

	for(list<RDUMMY*>::iterator i=m_DummyList.begin();i!=m_DummyList.end();++i)
	{
		RDUMMY *psp=*i;

		if( strnicmp( psp->name.c_str(), cszDummyName, strlen(cszDummyName))== 0 )
		{
			++ntest;
			aRootElement.AppendText("\n\t");

			MXmlElement SmokeElement = aRootElement.CreateChildElement(RTOK_SMOKE);

			strcpy( strBuffer, psp->name.c_str() );
			strcat( strBuffer, ".elu" );

			SmokeElement.AddAttribute(RTOK_SMOKE_NAME,strBuffer);

			if( psp->GetUserPropValue(RTOK_SMOKE_DIRECTION, &iBuffer) ) {
				SmokeElement.AddAttribute(RTOK_SMOKE_DIRECTION, iBuffer);
			}

			if( psp->GetUserPropValue(RTOK_SMOKE_POWER, &fBuffer) )
			{
				int up = (int)fBuffer;
				int down = (int)((fBuffer - up)*1000);
				sprintf( strBuffer, "%d.%d", up, down );
				SmokeElement.AddAttribute(RTOK_SMOKE_POWER, strBuffer);
			}

			if( psp->GetUserPropValue(RTOK_SMOKE_DELAY, &iBuffer) )
				SmokeElement.AddAttribute(RTOK_SMOKE_DELAY, iBuffer);

			if( psp->GetUserPropValue(RTOK_SMOKE_SIZE, &fBuffer) )
			{
				int up = (int)fBuffer;
				int down = (int)((fBuffer - up)*1000);
				sprintf( strBuffer, "%d.%d", up, down );
				SmokeElement.AddAttribute(RTOK_SMOKE_SIZE, strBuffer);
			}

			if( psp->GetUserPropValue(RTOK_SMOKE_COLOR, strBuffer,256) ) {
				SmokeElement.AddAttribute(RTOK_SMOKE_COLOR, strBuffer);
			}

			if( psp->GetUserPropValue(RTOK_SMOKE_LIFE, &fBuffer) )
			{
				int up = (int)fBuffer;
				int down = (int)((fBuffer - up)*1000);
				sprintf( strBuffer, "%d.%d", up, down );
				SmokeElement.AddAttribute(RTOK_SMOKE_LIFE, strBuffer);
			}

			if( psp->GetUserPropValue(RTOK_SMOKE_TOGMINTIME, &fBuffer) )
			{
				int up = (int)fBuffer;
				int down = (int)((fBuffer - up)*1000);
				sprintf( strBuffer, "%d.%d", up, down );
				SmokeElement.AddAttribute(RTOK_SMOKE_TOGMINTIME, strBuffer);
			}

			SmokeElement.AppendText("\t");

//			mlog("SaveSmoke 진입했음\n");
		}
	}

	if( ntest > 0 )
	{
		aRootElement.AppendText("\n");
		aXml.SaveToFile(name);
	}

	return true;
}

bool RSBspExporter::SaveFlag( const char* name )
{
	MXmlDocument	aXml;

	aXml.Create();
	aXml.CreateProcessingInstruction();

	MXmlElement		aRootElement;

	aRootElement=aXml.CreateElement("XML");
	aXml.AppendChild(aRootElement);

	const char* cszDummyName = "obj_flag";
	char strBuffer[256], strBuffer2[256];
	int		iBuffer, nTemp;
	float	fBuffer;

	int ntest = 0;
	
	for(list<RDUMMY*>::iterator i=m_DummyList.begin();i!=m_DummyList.end();++i)
	{
		RDUMMY *psp=*i;
		if( strnicmp( psp->name.c_str(), cszDummyName, strlen(cszDummyName))== 0 )
		{
			++ntest;
			aRootElement.AppendText("\n\t");
			MXmlElement FlagElement	= aRootElement.CreateChildElement(RTOK_FLAG);
			
			strcpy( strBuffer, psp->name.c_str() );
			strcat( strBuffer, ".elu" );

            FlagElement.AddAttribute(RTOK_FLAG_NAME,strBuffer);
						
			if( psp->GetUserPropValue(RTOK_FLAG_DIRECTION, &iBuffer) )
				FlagElement.AddAttribute(RTOK_FLAG_DIRECTION, iBuffer);
			if( psp->GetUserPropValue(RTOK_FLAG_POWER, &fBuffer) )
			{
				int up = (int)fBuffer;
				int down = (int)((fBuffer - up)*1000);
				sprintf( strBuffer, "%d.%d", up, down );
				FlagElement.AddAttribute(RTOK_FLAG_POWER, strBuffer);
			}

			if( psp->GetUserPropValue(RTOK_RESTRICTION, &nTemp ))
			{
				for( int i = 0; i < nTemp; ++i )
				{
					FlagElement.AppendText("\n\t\t");
					MXmlElement RestrictionElement = FlagElement.CreateChildElement(RTOK_RESTRICTION);
					sprintf(strBuffer2, "%s%d", RTOK_RESTRICTION_AXIS, i );
					log("%s\n",strBuffer2);
					if(psp->GetUserPropValue(strBuffer2, &iBuffer))
						RestrictionElement.AddAttribute(RTOK_RESTRICTION_AXIS, iBuffer);
					sprintf(strBuffer2, "%s%d", RTOK_RESTRICTION_POSITION, i );
					log("%s\n",strBuffer2);
					if(psp->GetUserPropValue(strBuffer2, &fBuffer))
					{
						int up = (int)fBuffer;
						int down = (int)(fabs((fBuffer - up)*1000));
						sprintf( strBuffer, "%d.%d", up, down );
						RestrictionElement.AddAttribute(RTOK_RESTRICTION_POSITION, strBuffer);
					}
					sprintf(strBuffer2, "%s%d", RTOK_RESTRICTION_COMPARE, i );
					log("%s\n",strBuffer2);
					if(psp->GetUserPropValue(strBuffer2, &iBuffer))
						RestrictionElement.AddAttribute(RTOK_RESTRICTION_COMPARE, iBuffer);
				}
			}

			if( psp->GetUserPropValue(RTOK_WINDTYPE, &iBuffer) )
			{
				FlagElement.AppendText("\n\t\t");
				MXmlElement WindElement = FlagElement.CreateChildElement("WINDTYPE");
				WindElement.AddAttribute(RTOK_WINDTYPE, iBuffer);
				if(psp->GetUserPropValue(RTOK_WINDDELAY, &iBuffer))
					WindElement.AddAttribute(RTOK_WINDDELAY, iBuffer);
			}
			FlagElement.AppendText("\n\t");
		}
	}

	if( ntest > 0 )
	{
		aRootElement.AppendText("\n");
		aXml.SaveToFile(name);
	}
	return true;
}

void VariantToString(const PROPVARIANT* pProp, TCHAR* szString, int bufSize)
{
	switch (pProp->vt) {
		case VT_LPWSTR:
			_tcscpy(szString, TSTR(pProp->pwszVal));
			break;
		case VT_LPSTR:
			_tcscpy(szString, TSTR(pProp->pszVal));
			break;
		case VT_I4:
			_stprintf(szString, "%ld", pProp->lVal);
			break;
		case VT_R4:
			_stprintf(szString, "%f", pProp->fltVal);
			break;
		case VT_R8:
			_stprintf(szString, "%lf", pProp->dblVal);
			break;
		case VT_BOOL:
			_stprintf(szString, "%s", pProp->boolVal ? "TRUE" : "FALSE");
			break;
		case VT_FILETIME:
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&pProp->filetime, &sysTime);
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
				DATE_SHORTDATE,
				&sysTime,
				NULL,
				szString,
				bufSize);
			break;
		default:
			_tcscpy(szString, "");	
			break;
	}
}

#define DEFAULT_FOG_MIN 1000
#define DEFAULT_FOG_MAX 10000
#define DEFAULT_FOG_COLOR_R 255
#define DEFAULT_FOG_COLOR_G 255
#define DEFAULT_FOG_COLOR_B 255

bool RSBspExporter::SaveFog(MXmlElement& Root)
{
	char szBuffer[256];
	int min =DEFAULT_FOG_MIN;
	int max =DEFAULT_FOG_MAX;
	int r = DEFAULT_FOG_COLOR_R;
	int g = DEFAULT_FOG_COLOR_G;
	int b = DEFAULT_FOG_COLOR_B;

	bool bFogEnable = false;

	int nProp = ip->GetNumProperties(PROPSET_USERDEFINED);
	if( nProp <= 0 ) return false;

	MXmlElement FogElement;

	for(int i = 0; i < nProp; ++i)
	{
		const PROPSPEC* prospec = ip->GetPropertySpec(PROPSET_USERDEFINED, i);
		const PROPVARIANT* provariant = ip->GetPropertyVariant(PROPSET_USERDEFINED, i);

		if (prospec->ulKind == PRSPEC_PROPID) 
			_stprintf(szBuffer, "%ld", prospec->propid);
		else 
			_tcscpy(szBuffer, TSTR(prospec->lpwstr));

		if(stricmp("fog_enable", szBuffer) == 0)
		{
			bFogEnable = true;
			if(!provariant->boolVal) return false;
			else
			{
				Root.AppendText("\n\t");
				FogElement = Root.CreateChildElement(RTOK_FOG);
			}
		}
		else if(stricmp("fog_min", szBuffer) == 0)
		{
			VariantToString(provariant, szBuffer, 256);
			min = atoi(szBuffer);
		}
		else if(stricmp("fog_max", szBuffer) == 0)
		{
			VariantToString(provariant, szBuffer, 256);
			max = atoi(szBuffer);
		}
		else if(stricmp("fog_color", szBuffer) == 0)
		{
			char* token;
			VariantToString(provariant, szBuffer, 256);
			token = strtok(szBuffer,",");
			if(token!=NULL) r=atoi(token);
			token = strtok(NULL,",");
			if(token!=NULL) g=atoi(token);
			token = strtok(NULL,",");
			if(token!=NULL) b=atoi(token);
		}	
	}	

	if (bFogEnable)
	{
		FogElement.AddAttribute("min", min);
		FogElement.AddAttribute("max", max);

		FogElement.AppendText("\n\t\t");
		MXmlElement r_elem = FogElement.CreateChildElement("R");
		r_elem.SetContents(r);

		FogElement.AppendText("\n\t\t");
		MXmlElement g_elem = FogElement.CreateChildElement("G");
		g_elem.SetContents(g);

		FogElement.AppendText("\n\t\t");
		MXmlElement b_elem = FogElement.CreateChildElement("B");
		b_elem.SetContents(b);

		FogElement.AppendText("\n\t");
	}

	return true;
}

bool RSBspExporter::SaveSoundProp( MXmlElement& Root )
{
	char szBuffer[256];
	const char* cszDummyName = "snd_amb";

	Root.AppendText("\n\t");
	MXmlElement ambSndElementList = Root.CreateChildElement("AMBIENTSOUNDLIST");

	for(list<RDUMMY*>::iterator i=m_DummyList.begin();i!=m_DummyList.end();++i)
	{
		D3DXVECTOR3 vec_min, vec_max;
		bool bmin, bmax;
		char* token = 0;

		RDUMMY *psp=*i;
		bmin = false;
		bmax = false;

		if( strnicmp( psp->name.c_str(), cszDummyName, strlen(cszDummyName))== 0 )
		{
			ambSndElementList.AppendText("\n\t\t");
			MXmlElement ambSndElement = ambSndElementList.CreateChildElement("AMBIENTSOUND");
			
			ambSndElement.AddAttribute("ObjName", psp->name.c_str());
			
            if(psp->GetUserPropValue("type",szBuffer,256))
			{
				ambSndElement.AddAttribute("type", szBuffer );
				if(szBuffer[1]=='1') // 구일 경우 중심을 표시
				{
					ambSndElement.AppendText("\n\t\t\t");
					MXmlElement posElement=ambSndElement.CreateChildElement("CENTER");
					posElement.SetContents(Format(szBuffer,psp->position));
				}
			}

			if(psp->GetUserPropValue("min_point",szBuffer,256))
			{
				token = strtok(szBuffer, ",");
				vec_max.x = -atof(token);
				token = strtok(NULL, ",");
				vec_min.y = atof(token);
				token = strtok(NULL, ",");
				vec_min.z = atof(token);

				bmin = true;				
			}
			if(psp->GetUserPropValue("max_point",szBuffer,256))
			{
				token = strtok(szBuffer, ",");
				vec_min.x = -atof(token);
				token = strtok(NULL, ",");
				vec_max.y = atof(token);
				token = strtok(NULL, ",");
				vec_max.z = atof(token);

				bmax = true;
			}
			if(psp->GetUserPropValue("r",szBuffer,256))
			{
				ambSndElement.AppendText("\n\t\t\t");
				MXmlElement posElement=ambSndElement.CreateChildElement("RADIUS");
				posElement.SetContents(szBuffer);	
			}
			if(psp->GetUserPropValue("file",szBuffer,256))
			{
				ambSndElement.AddAttribute("filename", szBuffer );
			}
			if( bmax & bmin )
			{
				ambSndElement.AppendText("\n\t\t\t");
				MXmlElement posElement=ambSndElement.CreateChildElement("MIN_POSITION");
				posElement.SetContents(Format(szBuffer,vec_min));

				ambSndElement.AppendText("\n\t\t\t");
				posElement=ambSndElement.CreateChildElement("MAX_POSITION");
				posElement.SetContents(Format(szBuffer,vec_max));	

				bmin = bmax = false;
			}
			
			ambSndElement.AppendText("\n\t\t");	
		}		
	}

	ambSndElementList.AppendText("\n\t");
	return true;
}

// xml에 <GLOBAL> 태그안에 Property들이 저장된다. 
// 맵의 전역적인 값을 설정할때 사용함.
bool RSBspExporter::SaveCustomProperties(MXmlElement& Root)
{
	char szBuffer[256];
	char szValue[1024];

	int nProp = ip->GetNumProperties(PROPSET_USERDEFINED);
	if( nProp <= 0 ) return false;

	Root.AppendText("\n\t");
	MXmlElement globalElement = Root.CreateChildElement(RTOK_GLOBAL);

	for(int i = 0; i < nProp; ++i)
	{
		const PROPSPEC* prospec = ip->GetPropertySpec(PROPSET_USERDEFINED, i);
		const PROPVARIANT* provariant = ip->GetPropertyVariant(PROPSET_USERDEFINED, i);

		if (prospec->ulKind == PRSPEC_PROPID) 
			_stprintf(szBuffer, "%ld", prospec->propid);
		else 
			_tcscpy(szBuffer, TSTR(prospec->lpwstr));

		VariantToString(provariant, szValue, 1024);

		char szTemp[256];
		sprintf(szTemp, "%s = %s\n", szBuffer, szValue);
		OutputDebugString("----\n");
		OutputDebugString(szTemp);



		globalElement.AppendText("\n\t\t");
		MXmlElement element1 = globalElement.CreateChildElement(szBuffer);
		element1.SetContents(szValue);

	}	
	globalElement.AppendText("\n\t");

	return true;
}

// for debug variables...
int nsplitcount=0,nleafcount=0;
int nbsppoly=0,nbspcpoly=0;

RSBspNode::RSBspNode()
{
	Positive=Negative=NULL;
	nFace=0;Face=NULL;
	bSolidNode=false;
}

RSBspNode::~RSBspNode()
{
	if(Negative) delete Negative;
	if(Positive) delete Positive;
	if(Face) delete []Face;
}

int RSBspNode::GetNodeCount()
{
	int nCount=1;

	if(Negative) nCount+=Negative->GetNodeCount();
	if(Positive) nCount+=Positive->GetNodeCount();

	return nCount;
}

int RSBspNode::GetPolygonCount()
{
	int nCount=nFace;

	if(Negative) nCount+=Negative->GetPolygonCount();
	if(Positive) nCount+=Positive->GetPolygonCount();

	return nCount;
}

int RSBspNode::GetIndicesCount()
{
	int nCount=0;

	for(int i=0;i<nFace;i++)
	{
		nCount+=(Face[i].nCount-2)*3;
	}

	if(Negative) nCount+=Negative->GetIndicesCount();
	if(Positive) nCount+=Positive->GetIndicesCount();

	return nCount;
}

int RSBspNode::GetVerticesCount()
{
	int nCount=0;

	for(int i=0;i<nFace;i++)
	{
		nCount+=Face[i].nCount;
	}

	if(Negative) nCount+=Negative->GetVerticesCount();
	if(Positive) nCount+=Positive->GetVerticesCount();

	return nCount;
}

RSBspNode *RSBspExporter::ConstructOctree(RSPolygonList *FaceList,int depth,dboundingbox *bb)
{
	RSBspNode *node=new RSBspNode;
	memcpy(&node->bbTree,bb,sizeof(dboundingbox));

	RSPolygonList faceNegList,facePosList;

	if(depth<g_nTreeDepth && FaceList->GetCount()>MAX_NODE_POLYGON_COUNT)	// 적당한 depth 에 도달하거나 개수가 적당해질때까지 나눈다.
	{
		if(depth-1<(int)m_PartitionPlaneVector.size())			// 미리 정의된 분할평면이 있다면...
		{
			node->plane=m_PartitionPlaneVector[depth-1];
			Partition(&node->plane,FaceList,&faceNegList,&facePosList);
			faceNegList.MoveFirst();facePosList.MoveFirst(); // for speed;
			node->Negative=faceNegList.GetCount()?ConstructOctree(&faceNegList,depth+1,&node->bbTree):NULL;
			node->Positive=facePosList.GetCount()?ConstructOctree(&facePosList,depth+1,&node->bbTree):NULL;
			ConstructBoundingBox(node);
		}else
		{
			dpoint diff=bb->vmax-bb->vmin;
			int nAxis =		diff.x > diff.y					 // 바운딩박스중 가장 큰 축을 찾는다..
				? diff.z > diff.x ? 2 : 0 
				: diff.z > diff.y ? 2 : 1;

			dpoint center = .5f*(bb->vmax+bb->vmin);

			dboundingbox bbpos,bbneg;

			memcpy(&bbpos,bb,sizeof(dboundingbox));
			bbpos.vmax[nAxis]=center[nAxis];

			memcpy(&bbneg,bb,sizeof(dboundingbox));
			bbneg.vmin[nAxis]=center[nAxis];

			dpoint normal=dpoint(0,0,0);
			normal[nAxis]=-1.f;

			DPlaneFromPointNormal(&node->plane,center,normal);		// 골라진 축으로 2등분하는 평면을 만든다.

			Partition(&node->plane,FaceList,&faceNegList,&facePosList);
			faceNegList.MoveFirst();facePosList.MoveFirst(); // for speed;
			node->Negative=faceNegList.GetCount()?ConstructOctree(&faceNegList,depth+1,&bbneg):NULL;
			node->Positive=facePosList.GetCount()?ConstructOctree(&facePosList,depth+1,&bbpos):NULL;
		}

	}
	else	// 더이상 쪼갤수 있는 폴리곤이 없으면, 이미 볼록다각형이며 leaf 노드로 만든다.
	{
		FaceList->MoveFirst();// for speed

		node->nFace=FaceList->GetCount();

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			memcpy(&node->Face[i],p,sizeof(rpolygon));
			p->v=NULL;	// 지울때 delete 하는걸 막는다
		}

		/*
		// 다각형을 삼각형으로 쪼갠다.

		int nFaceCount=0;
		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			nFaceCount+=p->nCount-2;
		}

		node->nFace=nFaceCount;

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);

			for(int j=0;j<p->nCount-2;j++)
			{
				memcpy(&node->Face[nIndex],p,sizeof(rpolygon));
				node->Face[nIndex].nCount=3;
				node->Face[nIndex].v=new rvertex[3];
				memcpy(&node->Face[nIndex].v[0],&p->v[0],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[1],&p->v[j+1],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[2],&p->v[j+2],sizeof(rvertex));
				nIndex++;
			}
		}
		*/

		nleafcount++;
		FaceList->DeleteAll();
	}

	return(node);
}

float fComplete;

RSBspNode *RSBspExporter::ConstructBspTree(RSPolygonList *FaceList)
{
	ip->ProgressUpdate(int(fComplete*100.f));
	if(ip->GetCancel()) return NULL;

	m_nDepth++;

	RSBspNode *node=new RSBspNode;

	RSPolygonList faceNegList,facePosList;

	if(ChoosePlane(FaceList,&node->plane))
	{
		Partition(&node->plane,FaceList,&faceNegList,&facePosList);
		faceNegList.MoveFirst();facePosList.MoveFirst(); // for speed;
		
		node->Negative=faceNegList.GetCount()?ConstructBspTree(&faceNegList):NULL;
		node->Positive=facePosList.GetCount()?ConstructBspTree(&facePosList):NULL;
	}
	else	// 더이상 쪼갤수 있는 폴리곤이 없으면, 이미 볼록다각형이며 leaf 노드로 만든다.

/*
	{
		FaceList->MoveFirst();// for speed
		// 다각형을 삼각형으로 쪼갠다.

		int nFaceCount=0;
		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			nFaceCount+=p->nCount-2;
		}

		node->nFace=nFaceCount;

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);

			for(int j=0;j<p->nCount-2;j++)
			{
				memcpy(&node->Face[nIndex],p,sizeof(rpolygon));
				node->Face[nIndex].nCount=3;
				node->Face[nIndex].v=new rvertex[3];
				memcpy(&node->Face[nIndex].v[0],&p->v[0],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[1],&p->v[j+1],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[2],&p->v[j+2],sizeof(rvertex));
				nIndex++;
			}
		}

		nleafcount++;
		FaceList->DeleteAll();

		float fProgress=pow(.5f,(m_nDepth-1));
		fComplete+=fProgress;
	}
	//*/

	{
		FaceList->MoveFirst();// for speed

		node->nFace=FaceList->GetCount();

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			memcpy(&node->Face[i],p,sizeof(rpolygon));
			p->v=NULL;	// 지울때 delete 하는걸 막는다
		}

		nleafcount++;
		FaceList->DeleteAll();
		
		float fProgress=pow(.5f,(m_nDepth-1));
		fComplete+=fProgress;
	}
	//*/

	m_nDepth--;
	return(node);
}

bool RSBspExporter::ChoosePlane(RSPolygonList *facelist,dplane *plane)
{
	float	fChosenScore=FLT_MAX;
	rpolygon *chosen=NULL;
	rpolygon *face;

	for(int i=0;i<facelist->GetCount();i++)
	{
		int nCount[5]={0,};		// 0,1 : pos/neg 수     2 : 쪼개지는 수		3,4 : 같은평면의 수 (pos/neg)
		face=facelist->Get(i);

		for(int j=0;j<facelist->GetCount();j++)
		{
			nCount[whichSideIsFaceWRTplane(facelist->Get(j),(dplane*)&face->normal)]++;
		}

		float fScore;
		if(m_nDepth<4)	
			// depth 가 이 이하일때는 밸런스 우선..
			fScore=1.f*abs(nCount[0]-nCount[1])+0.5f*nCount[2]+0.1f*(nCount[3]+nCount[4]);
		else
			// 이상일때는 쪼개지는 폴리곤이 없는 쪽 우선.
			fScore=0.5f*abs(nCount[0]-nCount[1])+1.f*nCount[2]+0.1f*(nCount[3]+nCount[4]);

		// 최소한 양쪽에 다 폴리곤이 있어야 하고 사용하지 않은 폴리곤이어야 한다
		int nPos=nCount[0]+nCount[3], nNeg=nCount[1]+nCount[4];
		if( nPos!=0 && nNeg!=0 && fScore<fChosenScore && !face->bPlaneUsed)
		{
			fChosenScore=fScore;
			chosen=face;
		}
	}
	if(chosen) {
		chosen->bPlaneUsed=true;
		memcpy(plane,&chosen->normal,sizeof(dplane));
	}
	return chosen!=NULL;
}

// 평면 선택의 기준을 balance & split 을 통합한 score 로 가자~!
bool RSBspExporter::ChoosePlaneSolid(RSPolygonList *facelist,dplane *plane)
{
	// front 나 back 한쪽으로 다 몰리더라도 모든 평면에 대해 쪼개야 한다 (solid bsp tree)
	float	fChosenScore=FLT_MAX;
	rpolygon *chosen=NULL;
	rpolygon *face;

	for(int i=0;i<facelist->GetCount();i++)
	{
		int nCount[5]={0,};		// 0,1 : pos/neg 수     2 : 쪼개지는 수		3,4 : 같은평면의 수
		face=facelist->Get(i);

		for(int j=0;j<facelist->GetCount();j++)
		{
			nCount[whichSideIsFaceWRTplane(facelist->Get(j),(dplane*)&face->normal)]++;
		}

		if(!face->bPlaneUsed)		// 이미 윗단계에서 사용한 폴리곤은 또 선택되지 않는다
		{
			// 각각의 factor 를 다르게 줄수 있다.. more balance / less split ...

			float fScore=1.f*abs(nCount[0]-nCount[1])+1.f*nCount[2]+.1f*(nCount[3]+nCount[4]);

			if( fScore<fChosenScore )
			{
				fChosenScore=fScore;
				chosen=face;
			}
		}
	}
	if(chosen) {
		chosen->bPlaneUsed=true;
		memcpy(plane,&chosen->normal,sizeof(dplane));
	}
	return chosen!=NULL;
}

#define MAX_WINDING_POINTS 1024

bool g_bSplitWorld=false;

void rpolygon::dump()
{
	_RPT1(_CRT_WARN,"%d - ",nID);
	for(int i=0;i<nCount;i++) {
		_RPT3(_CRT_WARN,"( %3.3f , %3.3f , %3.3f ) - ",coord(i)->x,coord(i)->y,coord(i)->z);
	}
	_RPT0(_CRT_WARN,"\n");
}

void SplitPolygon(rpolygon *f,dplane *plane,rpolygon **ppPosPoly,rpolygon **ppNegPoly)
{
//	_ASSERT(f->nID!=182);

	*ppNegPoly=NULL;
	*ppPosPoly=NULL;

	/*
	if(f->GetArea()<BSPTOLER) {
		delete f;
		return;
	}
	*/

	rsign signs[MAX_WINDING_POINTS];
	double dists[MAX_WINDING_POINTS];

	_ASSERT(f->nCount<MAX_WINDING_POINTS);

	int i,nPositive=0,nNegative=0,nZero=0;
	for(i=0;i<f->nCount;i++)
	{
		dists[i]=DPlaneDotCoord(*plane,f->v[i].coord);
		rsign sign=signof(dists[i]);

		signs[i]=sign;
		if(sign==POSITIVE)
			nPositive++;
		else
			if(sign==NEGATIVE)
				nNegative++;
			else
				nZero++;
	}
	signs[i]=signs[0];
	dists[i]=dists[0];


	if(nNegative==0 && nPositive==0)	// 쪼개는 평면위에 폴리곤이 놓여져있다
	{
		f->bPlaneUsed=true;			// 같은평면이면 사용했다고 check !

		if(DPlaneDotNormal(*plane,f->normal)>-0.9)
		{
			*ppPosPoly=f;
			return;
		}
		else
		{
			*ppNegPoly=f;
			return;
		}
	}

	if(nNegative==0)	// 모두 positive 쪽에 있으므로 poslist 로 넣는다.
	{
		*ppPosPoly=f;
		return;
	}

	if(nPositive==0)	// neglist 로..
	{
		*ppNegPoly=f;
		return;
	}

	// 양쪽에 걸친경우 쪼개야 한다

	int nPosVerCount=0,nNegVerCount=0;

	for(i=0;i<f->nCount;i++)
	{
		if(signs[i]!=POSITIVE)
			nNegVerCount++;
		if(signs[i]!=NEGATIVE)
			nPosVerCount++;

		if(signs[i]!=ZERO && signs[i+1]!=ZERO && signs[i]!=signs[i+1])
		{
			nNegVerCount++;
			nPosVerCount++;
		}
	}

	rpolygon *pNeg=new rpolygon;;
	memcpy(pNeg,f,sizeof(rpolygon));
	pNeg->nCount=nNegVerCount;
	pNeg->v=new rvertex[nNegVerCount];

	rpolygon *pPos=new rpolygon;;
	memcpy(pPos,f,sizeof(rpolygon));
	pPos->nCount=nPosVerCount;
	pPos->v=new rvertex[nPosVerCount];

	int nPosIndex=0,nNegIndex=0;

	for(i=0;i<f->nCount;i++)
	{
		if(signs[i]!=POSITIVE)
			memcpy(&pNeg->v[nNegIndex++],&f->v[i],sizeof(rvertex));

		if(signs[i]!=NEGATIVE)
			memcpy(&pPos->v[nPosIndex++],&f->v[i],sizeof(rvertex));

		if(signs[i]!=ZERO && signs[i+1]!=ZERO && signs[i]!=signs[i+1])
		{
			rvertex *v1=&f->v[i],*v2=&f->v[(i+1)%f->nCount];

			rvertex newv;

			double t;
			newv.coord=GetPlaneIntersectLine(*plane,v1->coord,v2->coord,&t);
			newv.normal=InterpolatedVector(v1->normal,v2->normal,t);

			newv.u=(float)(v1->u+(v2->u-v1->u)*t);
			newv.v=(float)(v1->v+(v2->v-v1->v)*t);

			_ASSERT((0<=t)&&(t<=1));

			memcpy(&pNeg->v[nNegIndex],&newv,sizeof(rvertex));
			memcpy(&pPos->v[nPosIndex],&newv,sizeof(rvertex));

			nNegIndex++;
			nPosIndex++;
		}
	}
	_ASSERT(nNegIndex==nNegVerCount && nPosIndex==nPosVerCount);

	*ppPosPoly=pPos;
	*ppNegPoly=pNeg;

	/*
	if(f->nID==182)
	{
		f->dump();
		_RPT4(_CRT_WARN,"plane [ %3.3f , %3.3f , %3.3f , %3.3f ] - ",plane->a,plane->b,plane->c,plane->d);

		_RPT0(_CRT_WARN,"pos --> ");
		pPos->dump();
		_RPT0(_CRT_WARN,"\n neg --> ");
		pPos->dump();
	}
	*/

	delete f;

	nsplitcount++;
}

rpolygon *NewLargePolygon(dplane &plane)
{
#define PORTALMAX 10000.f
#define PORTALMIN -10000.f

	rpolygon *p=new rpolygon(4);

	int nAxis;

	if(fabs(plane.a)>fabs(plane.b) && fabs(plane.a)>fabs(plane.c) )
		nAxis=0;
	else if(fabs(plane.b)>fabs(plane.c))
		nAxis=1;
	else
		nAxis=2;

	int nAxis1=(nAxis+1)%3,nAxis2=(nAxis+2)%3;

	double *fplane=(double*)plane;
	p->v[0].coord[nAxis1]=PORTALMIN;
	p->v[0].coord[nAxis2]=PORTALMIN;
	p->v[0].coord[nAxis]=-(fplane[nAxis1]*p->v[0].coord[nAxis1]+fplane[nAxis2]*p->v[0].coord[nAxis2]+plane.d)/fplane[nAxis];

	p->v[1].coord[nAxis1]=PORTALMAX;
	p->v[1].coord[nAxis2]=PORTALMIN;
	p->v[1].coord[nAxis]=-(fplane[nAxis1]*p->v[1].coord[nAxis1]+fplane[nAxis2]*p->v[1].coord[nAxis2]+plane.d)/fplane[nAxis];

	p->v[2].coord[nAxis1]=PORTALMAX;
	p->v[2].coord[nAxis2]=PORTALMAX;
	p->v[2].coord[nAxis]=-(fplane[nAxis1]*p->v[2].coord[nAxis1]+fplane[nAxis2]*p->v[2].coord[nAxis2]+plane.d)/fplane[nAxis];

	p->v[3].coord[nAxis1]=PORTALMIN;
	p->v[3].coord[nAxis2]=PORTALMAX;
	p->v[3].coord[nAxis]=-(fplane[nAxis1]*p->v[3].coord[nAxis1]+fplane[nAxis2]*p->v[3].coord[nAxis2]+plane.d)/fplane[nAxis];

	p->normal=dpoint(plane.a,plane.b,plane.c);
	p->d=plane.d;
	return p;
}

// plane 에 대해서 negative 쪽을 잘라낸다. (속도를위해서) 따로만드는게 낫다
rpolygon *ClipPolygon(rpolygon *f,dplane *plane)
{
	rpolygon *ppos,*pneg;
	SplitPolygon(f,plane,&ppos,&pneg);
	if(pneg) delete pneg;
	return ppos;
}

class RSolidVertexList : public list<dpoint> {
public:
	int Add(const dpoint &v) {
		int nIndex=0;
		for(iterator i=begin();i!=end();i++)
		{
			if(BSPEQ3(*i,v)) return nIndex;
			nIndex++;
		}
		push_back(v);
		return nIndex;
	}
};

RSBspNode *RSBspExporter::ConstructSolidBspTree(RSPolygonList *FaceList)
{
	ip->ProgressUpdate(int(fComplete*100.f));
	if(ip->GetCancel()) return NULL;

	m_nDepth++;

	if(FaceList->GetCount()==0) return NULL;

	RSBspNode *node=new RSBspNode;

	RSPolygonList faceNegList,facePosList;

	if(ChoosePlaneSolid(FaceList,&node->plane))
	{
		// 월드를 이루는 폴리곤을 나눈다
		Partition(&node->plane,FaceList,&faceNegList,&facePosList);
		faceNegList.MoveFirst();facePosList.MoveFirst(); // for speed;

		node->Positive=ConstructSolidBspTree(&facePosList);
		node->Negative=ConstructSolidBspTree(&faceNegList);

	}
	else	// 더이상 쪼갤수 있는 폴리곤이 없으면, 이미 볼록다각형이며 leaf 노드로 만든다.
	{
		FaceList->MoveFirst();// for speed
		// 다각형을 삼각형으로 쪼갠다.

		int nFaceCount=0;
		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			nFaceCount+=p->nCount-2;
		}

		node->nFace=nFaceCount;

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);

			for(int j=0;j<p->nCount-2;j++)
			{
				memcpy(&node->Face[nIndex],p,sizeof(rpolygon));
				node->Face[nIndex].nCount=3;
				node->Face[nIndex].v=new rvertex[3];
				memcpy(&node->Face[nIndex].v[0],&p->v[0],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[1],&p->v[j+1],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[2],&p->v[j+2],sizeof(rvertex));
				nIndex++;
			}
		}

		nleafcount++;
		FaceList->DeleteAll();

		float fProgress=pow(.5f,(m_nDepth-1));
		fComplete+=fProgress;
	}

	m_nDepth--;
	return(node);
}

float GetAngle(rvector &a)
{
	if(a.x>=1.0f) return 0.0f;
	if(a.x<=-1.0f) return -pi;
	if(a.y>0)
		return (float)acos(a.x);
	else
		return (float)-acos(a.x);
}

// convex hull의 단면 폴리곤을 구한다. split polygon 과 정확하게 똑같이 동작해야
// 폴리곤이 뜨지 않는다. (오차가 생기지 않도록 해야함)
rpolygon *GetPartitionPolygon(dplane &plane,RSPolygonList *ppl)
{
	RSolidVertexList ppv;

	for(int ip=0;ip<ppl->GetCount();)
	{
		rpolygon *f=ppl->Get(ip);

		rsign signs[MAX_WINDING_POINTS];
		double dists[MAX_WINDING_POINTS];

		_ASSERT(f->nCount<MAX_WINDING_POINTS);

		int i,nPositive=0,nNegative=0,nZero=0;

		for(i=0;i<f->nCount;i++)
		{
			dists[i]=DPlaneDotCoord(plane,f->v[i].coord);
			rsign sign=signof(dists[i]);

			signs[i]=sign;
			if(sign==POSITIVE)
				nPositive++;
			else
				if(sign==NEGATIVE)
					nNegative++;
				else
					nZero++;
		}

		if(nNegative==0 && nPositive==0) {		// 평면위의 폴리곤은 삭제
			ppl->Delete(ip);
			continue;
		}

		signs[i]=signs[0];
		dists[i]=dists[0];

		for(i=0;i<f->nCount;i++)
		{
			if(signs[i]==ZERO) {
				ppv.Add(f->v[i].coord);
			}else
			if(signs[i]!=ZERO && signs[i+1]!=ZERO && signs[i]!=signs[i+1])
			{
				rvertex *v1=&f->v[i],*v2=&f->v[(i+1)%f->nCount];
				double t;
				dpoint coord=GetPlaneIntersectLine(plane,v1->coord,v2->coord,&t);

				ppv.Add(coord);
			}
		}

		ip++;
	}

	// 이제 intersect 된 점들을 시계방향으로 묶어주면 된다.

	if(ppv.size()<3) return NULL;

	rpolygon *p=new rpolygon(ppv.size());

	{
		rvector center=rvector(0,0,0);

		for(RSolidVertexList::iterator i=ppv.begin();i!=ppv.end();i++){
			center.x+=(float)i->x;
			center.y+=(float)i->y;
			center.z+=(float)i->z;
		}

		center *= 1.f/float(p->nCount);

		rvector apoint=rvector((float)ppv.begin()->x,(float)ppv.begin()->y,(float)ppv.begin()->z);

		rvector up=center-apoint;
		rvector at=center-rvector((float)plane.a,(float)plane.b,(float)plane.c);

		rmatrix tm;
		D3DXMatrixLookAtLH(&tm,&center,&at,&up);

		map<float,dpoint> vertices;
		for(RSolidVertexList::iterator i=ppv.begin();i!=ppv.end();i++) {
			rvector tv,v;
			v=rvector((float)i->x,(float)i->y,(float)i->z);
			D3DXVec3TransformCoord(&tv,&v,&tm);
//			_ASSERT(fabs(tv.z)<BSPTOLER);
			Normalize(tv);
			float angle=GetAngle(tv);
			vertices.insert(map<float,dpoint>::value_type(angle,*i));
		}

		map<float,dpoint>::iterator it=vertices.begin();
		for(int j=0;j<p->nCount;j++) {
			p->v[j].coord=it->second;
			it++;
		}

		p->normal=dpoint(plane.a,plane.b,plane.c);
		p->d=plane.d;

	}
	return p;
}

RSBspNode *RSBspExporter::ConstructBevelSolidBspTree(RSPolygonList *FaceList,RSPolygonList *SpaceFaceList)
{
	ip->ProgressUpdate(int(fComplete*100.f));
	if(ip->GetCancel()) return NULL;

	if(FaceList->GetCount()==0) 
	{
		if(SpaceFaceList->GetCount()==0) {
			// 원래 이런경우는 없는데 -_-; 왜 나오나 ? -_-;
			return NULL;
		}

		// 폴리곤이 없으므로 solid leaf 노드이다.
		// 이제 공간을 이루는 폴리곤을 살펴보아 뾰족한 edge 나 vertex 를 찾아서 평면을 추가해 bevel 해준다.

		list<dplane> RBevelPlanes;

		int i,j,k,l;
		RSolidVertexList vertices;
		for(i=0;i<SpaceFaceList->GetCount();i++)
		{
			rpolygon *p=SpaceFaceList->Get(i);
//			p->en=new dpoint[p->nCount];
			p->vi=new int[p->nCount];
			for(j=0;j<p->nCount;j++)
			{
				p->vi[j]=vertices.Add(p->v[j].coord);
//				p->en[j]=p->normal;

//				log("(%3.5f %3.5f %3.5f) , ind= %d\n",p->v[j].coord.x,p->v[j].coord.y,p->v[j].coord.z,p->vi[j]);
			}
		}

		// 날카로운 edge 를 찾는다.
		for(i=0;i<SpaceFaceList->GetCount();i++)
		{
			rpolygon *p=SpaceFaceList->Get(i);
			for(j=0;j<p->nCount;j++)
			{
				for(k=i+1;k<SpaceFaceList->GetCount();k++)
				{
					rpolygon *p2=SpaceFaceList->Get(k);

					for(l=0;l<p2->nCount;l++)
					{
						// 인접한 edge 이면
						if((p->vi[(j+1)%p->nCount]==p2->vi[l] && p->vi[j]==p2->vi[(l+1)%p2->nCount]) ||
							(p->vi[j]==p2->vi[l] && p->vi[(j+1)%p->nCount]==p2->vi[(l+1)%p2->nCount]))
						{

#define EDGE_ANGLE		85.	// normal의 각도이므로 실제각은 95도
							// normal을 이루는 각도가 EDGE_ANGLE도 보다도 날카로운 edge 이면 추가한다
							if(DotProduct(p->normal,p2->normal)<cos(EDGE_ANGLE/pi)) {

								dpoint normal=p->normal+p2->normal;
								normal.Normalize();
								dpoint apoint=p->v[j].coord;

								dplane newplane;
								DPlaneFromPointNormal(&newplane,apoint,normal);
								RBevelPlanes.push_back(newplane);

								/*
								if(fabs(DPlaneDotCoord(newplane,p->v[(j+1)%p->nCount].coord))>0.01)
								{
									log("뷁! edge error!\n");
								}
								*/


								/*
								// (위에서 봤을때 외곽edge이면) 수직인 면을 하나 더 세운다 
								if(signof(p->normal.z)*signof(p2->normal.z)<0)
								{
									normal.z=0;
									normal.Normalize();
									DPlaneFromPointNormal(&newplane,apoint,normal);

									RSolidVertexList::iterator it=vertices.begin();
									for(i=0;i<(int)vertices.size();i++,it++)
									{
										dpoint apoint=*it;
										if(DPlaneDotCoord(newplane,apoint)<-0.01)
										{
											log("뷁! vertex error!\n");
										}
									}
									RBevelPlanes.push_back(newplane);
								}
								*/ // 예외경우가 있어서 봉인
							}

							/*
							// edge normal 을 써넣는다 ( 인접한 평면의 normal의 평균 )
							dpoint en=.5f*(p->normal+p2->normal);
							en.Normalize();

							p->en[j]=en;
							p2->en[l]=en;
							*/
						}
					}
				}
			}
		}

		/*
		// 중점을 구한다 
		dpoint center=dpoint(0,0,0);
		RSolidVertexList::iterator it=vertices.begin();
		for(i=0;i<(int)vertices.size();i++,it++)
		{
			center+=*it;
		}
		center*=1./(double)vertices.size();
		*/

		// 뾰족한 vertex 를 찾는다

		RSolidVertexList::iterator it=vertices.begin();
		for(i=0;i<(int)vertices.size();i++,it++)
		{
			dpoint avrnormal=dpoint(0,0,0);
			RSolidVertexList normals;
			
			map<double,dpoint>	areanormals;
			double	TotalArea=0.;

			// normals 에다가 사용된 폴리곤의 노말들을 모아놓는다
			for(j=0;j<SpaceFaceList->GetCount();j++)
			{
				rpolygon *p=SpaceFaceList->Get(j);
				for(k=0;k<p->nCount;k++)
				{
					if(p->vi[k]==i) {	// i 버텍스가 j 폴리곤에 사용되어지면
						normals.push_back(p->normal);

						// 중요 ! TODO : 각도에 의한 가중치로 다시코딩
						dpoint e1=*p->coord(k+1)-*p->coord(k),e2=*p->coord(k-1)-*p->coord(k);
						/*
						dpoint normal= CrossProduct(e1,e2)/(DotProduct(e1,e1)*DotProduct(e2,e2));
						avrnormal += normal;
						*/

						double costheta=DotProduct(e1,e2)/(e1.Length()*e2.Length());
						double theta=acos(costheta);
						avrnormal += theta*p->normal;

						
//						avrnormal+=p->normal;

						/*
						// k 버텍스를 포함하는 k , k-1 edge의 normal을 더한다
						avrnormal+=p->en[k];
						avrnormal+=p->en[(k+p->nCount-1)%p->nCount];
						*/

						/*
						dpoint normal;
						normal=Normalize(*p->coord(k)-*p->coord(k-1));
						avrnormal+=normal;
						normal=Normalize(*p->coord(k)-*p->coord(k+1));
						avrnormal+=normal;
						*/

						break;
					}
				}
			}


			// 모아놓은 normals 들을 비교해서 날카롭다고 판단되면 empty node를 추가한다
			bool bNeedToSplit=false;
			for(RSolidVertexList::iterator it1=normals.begin();it1!=normals.end();it1++)
			{
				RSolidVertexList::iterator it2=it1;
				it2++;
				for(;it2!=normals.end();it2++)
				{
					if(DotProduct(*it1,*it2)<0) {
						bNeedToSplit=true;
						break;
					}
				}
				if(bNeedToSplit) break;
			}

			if(bNeedToSplit) {

				/*
				dpoint normal;
				for(map<double,dpoint>::iterator i=areanormals.begin();i!=areanormals.end();i++)
				{
					normal+=(i->first/TotalArea)*i->second;
				}
				normal.Normalize();
				*/

				// 각도에 weight를 줘서 구하는 방법. avrnormal 은 이미 unit이나.. 정확도를 위해 다시한번.
				avrnormal.Normalize();

				dplane newplane;
				dpoint apoint=*it;

//				dpoint normal=Normalize(apoint-center);

				DPlaneFromPointNormal(&newplane,apoint,avrnormal);

				// 확인함 해보자

				
				for(RSolidVertexList::iterator itt=vertices.begin();itt!=vertices.end();itt++)
				{
					dpoint pt=*itt;
					double test=DPlaneDotCoord(newplane,pt);
					_ASSERT(test<0.01);
				}
				
				RBevelPlanes.push_back(newplane);
			}
		}

		RSBspNode *retnode=new RSBspNode;
		retnode->bSolidNode=true;
		retnode->Positive=NULL;
		retnode->Negative=NULL;

		int nFaceCount=0;
		for(i=0;i<SpaceFaceList->GetCount();i++)
		{
			rpolygon *p=SpaceFaceList->Get(i);
			nFaceCount+=p->nCount-2;
		}

		retnode->nFace=nFaceCount;

		int nIndex=0;
		retnode->Face=new rpolygon[retnode->nFace];

		for(i=0;i<SpaceFaceList->GetCount();i++)
		{
			rpolygon *p=SpaceFaceList->Get(i);

			int j;
			for(j=0;j<p->nCount-2;j++)
			{
				memcpy(&retnode->Face[nIndex],p,sizeof(rpolygon));
				retnode->Face[nIndex].nCount=3;
				retnode->Face[nIndex].v=new rvertex[3];
				memcpy(&retnode->Face[nIndex].v[0],&p->v[0],sizeof(rvertex));
				memcpy(&retnode->Face[nIndex].v[1],&p->v[j+1],sizeof(rvertex));
				memcpy(&retnode->Face[nIndex].v[2],&p->v[j+2],sizeof(rvertex));
				nIndex++;
			}

			/*
			for(j=0;j<p->nCount;j++)
			{
				_ASSERT(!BSPEQ3(p->v[j].coord,dpoint(0,0,0)));
			}
			*/
		}

		while(RBevelPlanes.size())
		{
			RSBspNode *pnode=new RSBspNode;
			pnode->nFace=0;
			pnode->Positive=NULL;
			pnode->Negative=retnode;
			pnode->plane=*RBevelPlanes.begin();
			retnode=pnode;
			RBevelPlanes.erase(RBevelPlanes.begin());
		}

		for(i=0;i<SpaceFaceList->GetCount();i++)
		{
			rpolygon *p=SpaceFaceList->Get(i);
			delete p->vi;
			p->vi=NULL;
		}

		return retnode;
	}

	RSBspNode *node=new RSBspNode;

	RSPolygonList faceNegList,facePosList;
	RSPolygonList posspaceFaceList,negspaceFaceList;

	if(ChoosePlaneSolid(FaceList,&node->plane))
	{
		g_bSplitWorld=true;
		// 월드를 이루는 폴리곤을 나눈다
		Partition(&node->plane,FaceList,&faceNegList,&facePosList);
		g_bSplitWorld=false;

		faceNegList.MoveFirst();facePosList.MoveFirst(); // for speed;


		// 공간을 이루는 폴리곤도 나눈다

		// 먼저 쪼개는 평면위의 폴리곤을 구한다.
		rpolygon *ppp=GetPartitionPolygon(node->plane,SpaceFaceList);

		Partition(&node->plane,SpaceFaceList,&negspaceFaceList,&posspaceFaceList);

		// 새로운 공간을 이루는 폴리곤을 추가한다 (positive쪽)

		/*
		int j;
		rpolygon *p=NewLargePolygon(-node->plane);
		for(j=0;j<m_nDepth;j++)
		{
			p=ClipPolygon(p,&g_planes[j]);
			if(!p) break;
		}
		if(p) posspaceFaceList.Add(p);
		*/

		if(ppp) negspaceFaceList.Add(ppp);
		if(ppp) {
			// negative 쪽은 폴리곤을 뒤집어서 넣는다.
			rpolygon *pppn=new rpolygon(ppp);
			for(int i=0;i<ppp->nCount;i++)
			{
				memcpy(&pppn->v[i],&ppp->v[ppp->nCount-i-1],sizeof(ppp->v[0]));
			}
			pppn->normal=-ppp->normal;
			pppn->d=-ppp->d;
			posspaceFaceList.Add(pppn);
		}


//		g_planes[m_nDepth]=node->plane;

		m_nDepth++;
		node->Positive=ConstructBevelSolidBspTree(&facePosList,&posspaceFaceList);
		m_nDepth--;

		// 새로운 공간을 이루는 폴리곤을 추가한다 (negative쪽)

		/*
		p=NewLargePolygon(node->plane);
		for(j=0;j<m_nDepth;j++)
		{
			p=ClipPolygon(p,&g_planes[j]);
			if(!p) break;
		}
		if(p) negspaceFaceList.Add(p);
		*/

//		g_planes[m_nDepth]=-node->plane;

		m_nDepth++;
		node->Negative=ConstructBevelSolidBspTree(&faceNegList,&negspaceFaceList);
		m_nDepth--;

	}
	else	// 더이상 쪼갤수 있는 폴리곤이 없으면, 이미 볼록다각형이며 leaf 노드로 만든다.
	{
		FaceList->MoveFirst();// for speed
		// 다각형을 삼각형으로 쪼갠다.

		int nFaceCount=0;
		for(int i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);
			nFaceCount+=p->nCount-2;

//			double fArea=p->GetArea();
		}

		node->nFace=nFaceCount;

		int nIndex=0;
		node->Face=new rpolygon[node->nFace];

		for(i=0;i<FaceList->GetCount();i++)
		{
			rpolygon *p=FaceList->Get(i);

			for(int j=0;j<p->nCount-2;j++)
			{
				memcpy(&node->Face[nIndex],p,sizeof(rpolygon));
				node->Face[nIndex].nCount=3;
				node->Face[nIndex].v=new rvertex[3];
				memcpy(&node->Face[nIndex].v[0],&p->v[0],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[1],&p->v[j+1],sizeof(rvertex));
				memcpy(&node->Face[nIndex].v[2],&p->v[j+2],sizeof(rvertex));
				nIndex++;
			}
		}

		/*
		rpolygon *p=FaceList->Get(0);
		if(Length(*p->coord(0)-dpoint(1655.4756,-2370.9888,-704.47247))<0.1
//			&& Length(*p->coord(1)-dpoint(1715.8564,-2370.9912,-704.47247))<0.1
//			&& Length(*p->coord(2)-dpoint(1655.4756,-2370.9919,-704.47247))<0.1
			)
			_ASSERT(FALSE);
		*/

		nleafcount++;
		FaceList->DeleteAll();

		float fProgress=pow(.5f,(m_nDepth-1));
		fComplete+=fProgress;
	}

	return(node);
}

// 어떻게 쪼개질지 알아보는 펑션이므로 partition & splitpolygon 과 같은 결과를 리턴해야한다
RSIDE RSBspExporter::whichSideIsFaceWRTplane(rpolygon *f,dplane *plane)
{
	int i,nPositive=0,nNegative=0,nZero=0;
	for(i=0;i<f->nCount;i++)
	{
		double dist=DPlaneDotCoord(*plane,f->v[i].coord);
		rsign sign=signof(dist);

		if(sign==POSITIVE)
			nPositive++;
		else
			if(sign==NEGATIVE)
				nNegative++;
			else
				nZero++;
	}

	if(nNegative==0 && nPositive==0)	// 쪼개는 평면위에 폴리곤이 놓여져있다
	{
		if(DPlaneDotNormal(*plane,f->normal)>-0.9)
		{
			return RSIDE_COPLANAR_POS;
		}
		else
		{
			return RSIDE_COPLANAR_NEG;
		}
	}

	if(nNegative==0)	// 모두 positive 쪽에 있으므로 poslist 로 넣는다.
		return RSIDE_POSITIVE;

	if(nPositive==0)	// neglist 로..
		return RSIDE_NEGATIVE;

	return RSIDE_BOTH;							
}

void RSBspExporter::Partition(dplane *plane,RSPolygonList *FaceList,
							RSPolygonList *NegList,RSPolygonList *PosList)
{
	rpolygon *face;
	FaceList->MoveFirst();
	while(FaceList->GetCount())
	{
		face=FaceList->Get();

		rpolygon *pPos,*pNeg;
		SplitPolygon(face,plane,&pPos,&pNeg);
		if(pPos) PosList->Add(pPos);
		if(pNeg) NegList->Add(pNeg);
		
		FaceList->DeleteRecord();
	}
}

void RSBspExporter::ConstructBoundingBox(RSBspNode *bspNode)
{
	if(bspNode->nFace)
	{
		dboundingbox *bb=&bspNode->bbTree;
		rpolygon *f=bspNode->Face;
		bb->vmin.x=bb->vmin.y=bb->vmin.z=FLT_MAX;
		bb->vmax.x=bb->vmax.y=bb->vmax.z=-FLT_MAX;
		for(int i=0;i<bspNode->nFace;i++)
		{
			for(int j=0;j<f->nCount;j++)
			{
				for(int k=0;k<3;k++)
				{
					bb->vmin[k]=min(bb->vmin[k],f->v[j].coord[k]);
					bb->vmax[k]=max(bb->vmax[k],f->v[j].coord[k]);
				}
			}
			f++;
		}
	}
	else
	{
		if(bspNode->Positive)
		{
			ConstructBoundingBox(bspNode->Positive);
			memcpy(&bspNode->bbTree,&bspNode->Positive->bbTree,sizeof(dboundingbox));
		}
		if(bspNode->Negative)
		{
			ConstructBoundingBox(bspNode->Negative);
			memcpy(&bspNode->bbTree,&bspNode->Negative->bbTree,sizeof(dboundingbox));
		}
		if(bspNode->Positive) MergeBoundingBox(&bspNode->bbTree,&bspNode->Positive->bbTree);
		if(bspNode->Negative) MergeBoundingBox(&bspNode->bbTree,&bspNode->Negative->bbTree);
	}
}

RSBspNode* RSBspExporter::GetLeafNode(dpoint *pos,RSBspNode *node)
{
	if(node->nFace) return node;
	if(node->plane.a*pos->x+node->plane.b*pos->y+node->plane.c*pos->z+node->plane.d>0)
		return GetLeafNode(pos,node->Positive);
	else
		return GetLeafNode(pos,node->Negative);
}

bool RSBspNode::Save(FILE *file)
{
//	fwrite(&bbTree,sizeof(dboundingbox),1,file);
	bbTree.SaveFloat(file);
	plane.SaveFloat(file);
//	fwrite(&plane,sizeof(dplane),1,file);

	bool flag;
	
	flag=(Positive!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Positive) Positive->Save(file);

	flag=(Negative!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Negative) Negative->Save(file);
	
	fwrite(&nFace,sizeof(int),1,file);
	for(int i=0;i<nFace;i++)
	{
		fwrite(&Face[i].nMaterial,sizeof(int),1,file);
		fwrite(&Face[i].nSourceIndex,sizeof(int),1,file);
		fwrite(&Face[i].dwFlags,sizeof(DWORD),1,file);

		fwrite(&Face[i].nCount,sizeof(int),1,file);
		
		for(int j=0;j<Face[i].nCount;j++)
		{
			Face[i].v[j].Save(file);
		}
		Face[i].normal.SaveFloat(file);

		/*
		Face[i].v[0].Save(file);
		Face[i].v[1].Save(file);
		Face[i].v[2].Save(file);
		*/
//		fwrite(&Face[i].v[0],sizeof(rvertex),1,file);
//		fwrite(&Face[i].v[1],sizeof(rvertex),1,file);
//		fwrite(&Face[i].v[2],sizeof(rvertex),1,file);
	}
	return true;
}

bool RSBspNode::SaveCol(FILE *file)
{
	plane.SaveFloat(file);
//	fwrite(&plane,sizeof(dplane),1,file);

	bool flag;

	// is solid ?
//	flag= (Positive==NULL && Negative==NULL && !bEmptyNode && nFace==0);
	fwrite(&bSolidNode,sizeof(bool),1,file);

	// is exist positive tree
	flag=(Positive!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Positive) Positive->SaveCol(file);

	// is exist negative tree
	flag=(Negative!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Negative) Negative->SaveCol(file);

	// 폴리곤정보. 이건 나중에 없애야함.
	fwrite(&nFace,sizeof(int),1,file);
	for(int i=0;i<nFace;i++)
	{
		Face[i].v[0].coord.SaveFloat(file);
		Face[i].v[1].coord.SaveFloat(file);
		Face[i].v[2].coord.SaveFloat(file);
		Face[i].normal.SaveFloat(file);

		/*
		_ASSERT(!BSPEQ3(Face[i].v[0].coord,dpoint(0,0,0)));
		_ASSERT(!BSPEQ3(Face[i].v[1].coord,dpoint(0,0,0)));
		_ASSERT(!BSPEQ3(Face[i].v[2].coord,dpoint(0,0,0)));
		*/
//		fwrite(&Face[i].v[0].coord,sizeof(dpoint),1,file);
//		fwrite(&Face[i].v[1].coord,sizeof(dpoint),1,file);
//		fwrite(&Face[i].v[2].coord,sizeof(dpoint),1,file);

		/*
		_ASSERT(fabs(Face[i].v[0].coord.x-1655)>1.f);
		_RPT1(_CRT_WARN,"%3.3f \n",Face[i].v[0].coord.x);
		if(Magnitude(rvector(Face[0].v[0].coord.x,Face[0].v[0].coord.y,Face[0].v[0].coord.z)
			-rvector(1655.4756f,-2370.9888f,-704.47247f))<10
			//			&& Length(*p->coord(1)-dpoint(1715.8564,-2370.9912,-704.47247))<0.1
			//			&& Length(*p->coord(2)-dpoint(1655.4756,-2370.9919,-704.47247))<0.1
			)
			_ASSERT(FALSE);
		*/

	}

	return true;
}

bool RSBspNode::SaveNavigation(FILE* file)
{
	plane.SaveFloat(file);

	bool flag;

	// is solid ?
	fwrite(&bSolidNode,sizeof(bool),1,file);

	// is exist positive tree
	flag=(Positive!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Positive) Positive->SaveCol(file);

	// is exist negative tree
	flag=(Negative!=NULL);
	fwrite(&flag,sizeof(bool),1,file);
	if(Negative) Negative->SaveCol(file);

	// face 정보.
	fwrite(&nFace,sizeof(int),1,file);
	for(int i=0;i<nFace;i++)
	{
		Face[i].v[0].coord.SaveFloat(file);
		Face[i].v[1].coord.SaveFloat(file);
		Face[i].v[2].coord.SaveFloat(file);
		Face[i].normal.SaveFloat(file);
	}

	return true;
}

bool RSBspExporter::Save(FILE *file)
{
	if(!OcHead) return false;

	{	// 나중에 이 정보를 읽어서 맞는 bsp 파일인지 확인한다.
		int nNodeCount=BspHead->GetNodeCount();
		fwrite(&nNodeCount,sizeof(int),1,file);
		int nPolygonCount=BspHead->GetPolygonCount();
		fwrite(&nPolygonCount,sizeof(int),1,file);
		int nVerticesCount=BspHead->GetVerticesCount();
		fwrite(&nVerticesCount,sizeof(int),1,file);
		int nIndicesCount=BspHead->GetIndicesCount();
		fwrite(&nIndicesCount,sizeof(int),1,file);
	}

	int nNodeCount=OcHead->GetNodeCount();
	fwrite(&nNodeCount,sizeof(int),1,file);
	int nPolygonCount=OcHead->GetPolygonCount();
	fwrite(&nPolygonCount,sizeof(int),1,file);
	int nVerticesCount=OcHead->GetVerticesCount();
	fwrite(&nVerticesCount,sizeof(int),1,file);
	int nIndicesCount=OcHead->GetIndicesCount();
	fwrite(&nIndicesCount,sizeof(int),1,file);

	bool ret=OcHead->Save(file);
	
	if(!ret) return false;

	return true;
}

bool RSBspExporter::SaveBSP(const char *name)
{
	if(!BspHead) return false;

	FILE *file=fopen(name,"wb+");
	if(!file) return false;

	RHEADER header(RBSP_ID,RBSP_VERSION);
	fwrite(&header,sizeof(RHEADER),1,file);

	int nNodeCount=BspHead->GetNodeCount();
	fwrite(&nNodeCount,sizeof(int),1,file);
	int nPolygonCount=BspHead->GetPolygonCount();
	fwrite(&nPolygonCount,sizeof(int),1,file);
	int nVerticesCount=BspHead->GetVerticesCount();
	fwrite(&nVerticesCount,sizeof(int),1,file);
	int nIndicesCount=BspHead->GetIndicesCount();
	fwrite(&nIndicesCount,sizeof(int),1,file);
	bool ret=BspHead->Save(file);
	fclose(file);

	if(!ret) return false;

	return true;
}

bool RSBspExporter::SaveCol(const char* name)
{
	if(!ColBspRoot) return false;

	FILE *file=fopen(name,"wb+");
	if(!file) return false;

	RHEADER header(R_COL_ID,R_COL_VERSION);
	fwrite(&header,sizeof(RHEADER),1,file);

	int nNodeCount=ColBspRoot->GetNodeCount();
	fwrite(&nNodeCount,sizeof(int),1,file);
	int nPolygonCount=ColBspRoot->GetPolygonCount();
	fwrite(&nPolygonCount,sizeof(int),1,file);
	bool ret=ColBspRoot->SaveCol(file);
	fclose(file);

	if(!ret) return false;

	return true;
}

bool RSBspExporter::SaveNavigation(const char* name)
{
	if (m_Navigation.nVertCount <= 0) return false;


	RNavigationMesh nm;
	nm.InitVertices(m_Navigation.nVertCount);
	for (int i = 0; i < m_Navigation.nVertCount; i++)
	{
		nm.SetVertex(i, m_Navigation.vertices[i]);
	}
	nm.InitFaces(m_Navigation.nFaceCount);
	for (int i = 0; i < m_Navigation.nFaceCount; i++)
	{
		nm.SetFace(i, m_Navigation.faces[i]);
	}

	nm.Save(name);

	return true;
}

RSPolygonList *pcurrentlist=NULL;

void RSBspExporter::CutPolygon(rpolygon *ppolygon)
{
	int i;

	/*
	// debug dump 
	_RPT0(_CRT_WARN,"cutting polygon - ");
	for(i=0;i<ppolygon->nCount;i++)
	{
		_RPT3(_CRT_WARN,"( %3.3f , %3.3f , %3.3f ) - ",ppolygon->v[i].coord.x,ppolygon->v[i].coord.y,ppolygon->v[i].coord.z);
	}
	_RPT0(_CRT_WARN,"\n");
	*/

	dplane plane=ppolygon->GetPlane();
	RSPolygonList *pcutlist=new RSPolygonList;
	// 같은 평면에 있는 폴리곤을 일단 cutlist 로 옮긴다
	for(i=0;i<pcurrentlist->GetCount();)
	{
		rpolygon *pp=pcurrentlist->Get(i);
		RSIDE side=whichSideIsFaceWRTplane(pp,&plane);
//		if(BSPEQ3(pp->normal,ppolygon->normal) && BSPEQ(pp->d,ppolygon->d))
		if(side==RSIDE_COPLANAR_POS || side==RSIDE_COPLANAR_NEG)
		{
			pcutlist->Add(pp);
			pcurrentlist->DeleteRecord(i);
		}
		else
			i++;
	}

	// 각각의 edge 에 대해 평면을 세운뒤 그것으로 잘라낸다
	for(i=0;i<ppolygon->nCount;i++)
	{
		dpoint apoint=ppolygon->v[i].coord;
//		dpoint edge=apoint-ppolygon->v[(i+1)%ppolygon->nCount].coord;
		dpoint edge=apoint-*ppolygon->coord(i+1);

		dpoint normal=CrossProduct(edge,ppolygon->normal);
		normal.Normalize();

		dplane edgeplane;
		DPlaneFromPointNormal(&edgeplane,apoint,normal);


		RSPolygonList *newcutlist=new RSPolygonList;
		
		pcutlist->MoveFirst();
		while(pcutlist->GetCount())
		{
			rpolygon *ps=pcutlist->Get();
			int nSourceID=ps->nID;
			rpolygon *pPos,*pNeg;
			SplitPolygon(ps,&edgeplane,&pPos,&pNeg);
			pcutlist->DeleteRecord();

			if(pPos) {
				pPos->nID=nSourceID;

				RSIDE test=whichSideIsFaceWRTplane(pPos,&edgeplane);

				if(test==RSIDE_COPLANAR_POS || test==RSIDE_COPLANAR_NEG)
					delete pPos;
				else
					pcurrentlist->Add(pPos);

//				_ASSERT(test!=RSIDE_COPLANAR_POS && test!=RSIDE_COPLANAR_NEG);
			}
			if(pNeg) {
				pNeg->nID=nSourceID;

				RSIDE test=whichSideIsFaceWRTplane(pNeg,&edgeplane);
				if(test==RSIDE_COPLANAR_POS || test==RSIDE_COPLANAR_NEG)
					delete pNeg;
				else
					newcutlist->Add(pNeg);

//				_ASSERT(test!=RSIDE_COPLANAR_POS && test!=RSIDE_COPLANAR_NEG);
			}
		}

		delete pcutlist;
		pcutlist=newcutlist;
	}

	delete pcutlist;			// 마지막까지 남는 넘들이 정말 버리는 부분이다
}

RCSGObject::RCSGObject()
{
	m_pPolygons=NULL;
	m_pTree=NULL;
}

RCSGObject::~RCSGObject()
{
	SAFE_DELETE(m_pPolygons);
	SAFE_DELETE(m_pTree);
}

void RSBspExporter::CheckPolygonInSolid(rpolygon *ppolygon,RSBspNode *pNode)
{
	if(!ppolygon) return;
	if(!pNode)
	{
		CutPolygon(ppolygon);
		return;
	}

	if(!pNode->Positive && !pNode->Negative)	// 말단노드이면
	{
		if(pNode->nFace==0)
			CutPolygon(ppolygon);
		return;
	}

	rpolygon *pPos,*pNeg;

	SplitPolygon(ppolygon,&pNode->plane,&pPos,&pNeg);

	CheckPolygonInSolid(pPos,pNode->Positive);
	CheckPolygonInSolid(pNeg,pNode->Negative);
}

#include <set>

bool RSBspExporter::CSGUnion(RSPolygonList *pOutputPolygons, RSPolygonList* pOutputPolygonList, list<RCSGObject*>& Objects)
{
	// 일단 각각의 object 들에 대해 bsptree를 생성한다
	for(list<RCSGObject*>::iterator i=Objects.begin();i!=Objects.end();i++){
		fComplete=0.f;
		m_nDepth=0;
		nsplitcount=0;nleafcount=0;

		RCSGObject *pc1=*i;

		// 각 object의 bsp트리를 만들기 위해 여벌의 facelist 를 만든다.
		RSPolygonList pl;
		for(int i=0;i<pc1->m_pPolygons->GetCount();i++)
		{
			rpolygon *f=new rpolygon(pc1->m_pPolygons->Get(i));
			pl.Add(f);
		}
		log("construct object subtree %s ( %d source polygon ) : ",pc1->m_Name.c_str(),pl.GetCount());
		
		pc1->m_pTree=ConstructSolidBspTree(&pl);
		log("total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
			nsplitcount,nleafcount,pc1->m_pTree->GetNodeCount(),pc1->m_pTree->GetPolygonCount());

	}
	

	for(list<RCSGObject*>::iterator i=Objects.begin();i!=Objects.end();i++){
		RCSGObject *pc1=*i;
		pcurrentlist=pc1->m_pPolygons;

		// 각 폴리곤에 대해 solid 안에 들어가있는지 체크하기 위해 다시 여벌의 facelist 를 만든다.
		RSPolygonList pl;
		for(int i=0;i<pc1->m_pPolygons->GetCount();i++)
		{
			rpolygon *f=new rpolygon(pc1->m_pPolygons->Get(i));
			pl.Add(f);
		}

		for(int j=0;j<pl.GetCount();j++) {
			rpolygon *ppolygon=pl.Get(j);

			for(list<RCSGObject*>::iterator k=Objects.begin();k!=Objects.end();k++){
				RCSGObject *pc2=*k;

				if(pc2==pc1) continue;

				rpolygon *f=new rpolygon(ppolygon);
				CheckPolygonInSolid(f,pc2->m_pTree);
			}
		}
	}

	// 서로에 대해 solid 안에 들어가 있는 폴리곤들이 제거되었다.
	// 이제 완전히 제거된 폴리곤을 원본에서 삭제한다

	int nSolidFaces=0;
	{
		int i,j;

		set<int> ids;
		for(list<RCSGObject*>::iterator it=Objects.begin();it!=Objects.end();it++){
			RCSGObject *pc=*it;
			for(j=0;j<pc->m_pPolygons->GetCount();j++)
			{
//				_ASSERT(pc->m_pPolygons->Get(j)->nID!=182);
				ids.insert(pc->m_pPolygons->Get(j)->nID);
			}
		}

		for(i=0;i<face.GetCount();i++)
		{
			rpolygon *pp=face.Get(i);

			// additive 나 opacitymap 쓰는 폴리곤 혹은 pass 인 넘들은 제거하지않는다
			if(((pp->dwFlags && ( RM_FLAG_PASSTHROUGH | RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY ))==NULL) 
				&& (ids.find(i)==ids.end()))
			{
				nSolidFaces++;
				delete pp;
			}else
				pOutputPolygons->Add(pp);
		}

		face.DeleteRecordAll();
	}

	log("%d faces eliminated. (solid)\n",nSolidFaces);

	RSPolygonList col;
	{
		for(list<RCSGObject*>::iterator i=Objects.begin();i!=Objects.end();i++)
		{
			RCSGObject *pObject=*i;
			pObject->m_pPolygons->MoveFirst();
			while(pObject->m_pPolygons->GetCount())
			{
				col.Add(pObject->m_pPolygons->Get());
				pObject->m_pPolygons->DeleteRecord();
			}
		}
	}

	log("%d source polygons remain\n",col.GetCount());

	MergePolygons(&col,pOutputPolygonList);
	log("optimized %d polygons remain\n",pOutputPolygonList->GetCount());

	/*
	RConvexPolygons convexpolygons;
	// 일단 볼록다각형으로 합칠수 있는데까지 합쳐본다..
	{

		int i,j,nStartIndex=0,nCount=faceCol.GetCount();
		bool *isdone=new bool[faceCol.GetCount()];
		for(i=0;i<faceCol.GetCount();i++)
			isdone[i]=false;

		for(i=0;i<nCount;i++)
		{
			if(!isdone[i])	
			{
				// 이 폴리곤이 아직 추가안되었으면 같은 평면에 있는 같은 material의 폴리곤들을 집어넣는다.
				rpolygon *f=faceCol.Get(i);
				//_ASSERT(f->nCount==3);

				dplane plane=dplane(f->normal.x,f->normal.y,f->normal.z,f->d);
				
//				_ASSERT(!BSPEQ(720.313,f->d));

				for(j=i;j<nCount;j++)
				{
					rpolygon *f2=faceCol.Get(j);

					int k;
					bool bCoplanar=true;
					for(k=0;k<f2->nCount;k++)
					{
						dpoint v=dpoint(f2->v[k].coord);
						double fDist=DPlaneDotCoord(plane,v);
						if(signof(fDist)!=ZERO) {
							bCoplanar=false;
							break;
						}
					}

					if(bCoplanar && f->nMaterial==f2->nMaterial)
					{
						RConvexPolygon *pPolygon=new RConvexPolygon;
						for(k=0;k<f2->nCount;k++)
						{
							pPolygon->vertices.push_back(new dpoint(f2->v[k].coord));
							pPolygon->normals.push_back(new dpoint(f2->v[k].normal));
						}

						pPolygon->fArea=0;
						for(k=0;k<f2->nCount-2;k++)
							pPolygon->fArea+=GetArea(f2->v[0].coord,f2->v[k+1].coord,f2->v[k+2].coord);

						if(DPlaneDotNormal(plane,f2->normal)>0)
							pPolygon->plane=plane;
						else
							pPolygon->plane=-plane;
						pPolygon->nMaterial=f->nMaterial;
						pPolygon->dwFlags=f->dwFlags;
						pPolygon->RefIDs.push_back(j);
						//pPolygon->faces.push_back(f2);
						convexpolygons.push_back(pPolygon);
						isdone[j]=true;
					}
				}

				// 같은 평면의 폴리곤을 다 집어 넣었으면 그 안에서 볼록다각형이 될만한것들을 찾아서 합쳐준다.
				convexpolygons.MergePolygons(nStartIndex);
				nStartIndex=convexpolygons.size();
			}
		}
	}

	log("%d collision convex polygons remain\n",convexpolygons.size());

	{
		faceCol.DeleteAll();
		for(size_t i=0;i<convexpolygons.size();i++)
		{
			RConvexPolygon *pPolygon=convexpolygons.at(i);
			rpolygon *pp=new rpolygon(pPolygon->vertices.size());
			for(int j=0;j<pp->nCount;j++)
			{
				pp->v[j].coord=*pPolygon->vertices.at(j);
			}
			pp->normal=dpoint(pPolygon->plane.a,pPolygon->plane.b,pPolygon->plane.c);
			pp->d=pPolygon->plane.d;

			faceCol.Add(pp);
		}
	}
	*/

	return true;
}


rpolygon *NewPolygonFromBoundingBox(dboundingbox &bb,int i)
{
	int nAxis=i/2;
	double fCoordAxis= (i%2) ? bb.vmin[nAxis] : bb.vmax[nAxis];

	int nAxis1=(nAxis+1)%3,nAxis2=(nAxis+2)%3;

	rpolygon *p= new rpolygon(4);

	if(i%2) {
		p->v[3].coord[nAxis]=fCoordAxis;
		p->v[3].coord[nAxis1]=bb.vmin[nAxis1];
		p->v[3].coord[nAxis2]=bb.vmin[nAxis2];

		p->v[2].coord[nAxis]=fCoordAxis;
		p->v[2].coord[nAxis1]=bb.vmin[nAxis1];
		p->v[2].coord[nAxis2]=bb.vmax[nAxis2];

		p->v[1].coord[nAxis]=fCoordAxis;
		p->v[1].coord[nAxis1]=bb.vmax[nAxis1];
		p->v[1].coord[nAxis2]=bb.vmax[nAxis2];

		p->v[0].coord[nAxis]=fCoordAxis;
		p->v[0].coord[nAxis1]=bb.vmax[nAxis1];
		p->v[0].coord[nAxis2]=bb.vmin[nAxis2];
	}else
	{
		p->v[0].coord[nAxis]=fCoordAxis;
		p->v[0].coord[nAxis1]=bb.vmin[nAxis1];
		p->v[0].coord[nAxis2]=bb.vmin[nAxis2];

		p->v[1].coord[nAxis]=fCoordAxis;
		p->v[1].coord[nAxis1]=bb.vmin[nAxis1];
		p->v[1].coord[nAxis2]=bb.vmax[nAxis2];

		p->v[2].coord[nAxis]=fCoordAxis;
		p->v[2].coord[nAxis1]=bb.vmax[nAxis1];
		p->v[2].coord[nAxis2]=bb.vmax[nAxis2];

		p->v[3].coord[nAxis]=fCoordAxis;
		p->v[3].coord[nAxis1]=bb.vmax[nAxis1];
		p->v[3].coord[nAxis2]=bb.vmin[nAxis2];
	}

	p->normal=dpoint(0,0,0);
	p->normal[nAxis]= (i%2) ? -1 : 1;
	p->d=-DotProduct(p->normal,p->v[0].coord);

	return p;
}

bool RSBspExporter::ConstructBspTree()
{
	// collision bsp tree 를 생성한다.

	log("Constructing Collision BspTree. ( %d source polygons )\n",faceCol.GetCount());

	fComplete=0.f;
	m_nDepth=0;
	nsplitcount=0;nleafcount=0;
	
	// collision 의 바운딩박스를 찾는다
	dboundingbox bbCol;
	bbCol.vmin.x=bbCol.vmin.y=bbCol.vmin.z=FLT_MAX;
	bbCol.vmax.x=bbCol.vmax.y=bbCol.vmax.z=-FLT_MAX;

	for(int i=0;i<faceCol.GetCount();i++)
	{
		rpolygon *f=faceCol.Get(i);
		for(int j=0;j<f->nCount;j++)
		{
			for(int k=0;k<3;k++)
			{
				bbCol.vmin[k]=min(bbCol.vmin[k],f->v[j].coord[k]);
				bbCol.vmax[k]=max(bbCol.vmax[k],f->v[j].coord[k]);
			}
		}
	}

	// solid노드를 이룰 박스 메쉬를 만든다
	RSPolygonList spacepolygons;
	for(int i=0;i<6;i++)
	{
		rpolygon *p=NewPolygonFromBoundingBox(bbCol,i);
		spacepolygons.Add(p);
	}

	ColBspRoot=ConstructBevelSolidBspTree(&faceCol,&spacepolygons);

	if(ColBspRoot)
	log("Collision Bsp Info : total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
		nsplitcount,nleafcount,ColBspRoot->GetNodeCount(),ColBspRoot->GetPolygonCount());

/*
	if (!m_NavObjects.empty())
	{
		m_pNavBspRoot=ConstructBevelSolidBspTree(&faceNavigation, &spacepolygons);
		if (m_pNavBspRoot)
		{
			log("Navigation Mesh Bsp Info: total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
				nsplitcount, nleafcount, m_pNavBspRoot->GetNodeCount(), 
				m_pNavBspRoot->GetPolygonCount());
		}
	}
*/

	// bsptree 를 생성하기 위해 여벌의 facelist 를 만든다.
	for(int i=0;i<faceOc.GetCount();i++)
	{
		rpolygon *fs=faceOc.Get(i);
		rpolygon *f=new rpolygon(fs);
		face.Add(f);
	}

	// partition plane vector 를 생성 (sort)
	for(map<int,dplane>::iterator i=m_PartitionPlanes.begin();i!=m_PartitionPlanes.end();i++)
	{
		m_PartitionPlaneVector.push_back(i->second);
	}

	// Octree 를 생성합니다.
	log("Constructing OcTree. ( %d source polygons )\n",faceOc.GetCount());
	nsplitcount=0;nleafcount=0;
	OcHead=ConstructOctree(&faceOc,1,&m_bb);
	log("Octree Info : total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
		nsplitcount,nleafcount,OcHead->GetNodeCount(),OcHead->GetPolygonCount());

	// bsp tree 를 생성한다.
	log("Constructing BspTree. ( %d source polygons )\n",face.GetCount());
	fComplete=0.f;
	m_nDepth=0;
	nsplitcount=0;nleafcount=0;
	BspHead=ConstructBspTree(&face);
	log("RSBsp Info : total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
		nsplitcount,nleafcount,BspHead->GetNodeCount(),BspHead->GetPolygonCount());

	log("Constructing Bsp Bounding Volumes.\n");
	ConstructBoundingBox(BspHead);

	/*
	// collision bsp tree 를 생성한다.
	fComplete=0.f;
	m_nDepth=0;
	nsplitcount=0;nleafcount=0;
	ColBspRoot=ConstructBspTree(&faceCol);

	log("Collision Bsp Info : total %d faces splitted %d leaf nodes %d nodes %d polygons\n",
		nsplitcount,nleafcount,ColBspRoot->GetNodeCount(),ColBspRoot->GetPolygonCount());
*/
	return true;
}

// nStartIndex 부터 끝까지 폴리곤중에서 합칠수 있는것들을 합친다.
// 이 폴리곤들은 모두 한평면위에 있다.
/*
void RSBspExporter::RConvexPolygons::MergePolygons(int nStartIndex)	
{
	dplane plane=at(nStartIndex)->plane;

	int nMergeCount=0;

	size_t i=nStartIndex,j,k,l;

	while(i<size())
	{
		RConvexPolygon *poly=(*this)[i];

		dpoint *edgea,*edgeb;

		j=0;
		while(j<poly->vertices.size())
		{
			bool bMerged=false;

			edgea=poly->vertices.Get(j);
			edgeb=poly->vertices.Get(j+1);

			for(k=i+1;k<size();)
			{
				RConvexPolygon *poly2=(*this)[k];

				if(poly->dwFlags==poly2->dwFlags)				// 플래그가 같아야 합쳐진다 (nopath..cast/receive shadow 등등)
				{
					for(l=0;l<poly2->vertices.size();l++)
					{
						dpoint *edge2a,*edge2b;
						edge2a=poly2->vertices.Get(l);
						edge2b=poly2->vertices.Get(l+1);
						
						if((BSPEQ3(*edge2a,*edgeb) && BSPEQ3(*edge2b,*edgea)) 
						//	|| (IS_EQ3(*edge2a,*edgeb) && IS_EQ3(*edge2b,*edgea))
							)				// 맞는 edge 를 찾았다 !
						{
							// normal 이 같은지 확인
							dpoint n1=*poly->normals.Get(j+1);
							dpoint n2=*poly2->normals.Get(l);
							_ASSERT(BSPEQ3(n1,n2));
							n1=*poly->normals.Get(j);
							n2=*poly2->normals.Get(l+1);
							_ASSERT(BSPEQ3(n1,n2));

							dpoint normal;
							dplane testplane;
							normal=CrossProduct(*poly->vertices.Get(j)-*poly->vertices.Get(j-1),dpoint(poly->plane.a,poly->plane.b,poly->plane.c));
							normal.Normalize();
							DPlaneFromPointNormal(&testplane,*poly->vertices.Get(j),normal);
							double fDetHead=DPlaneDotCoord(testplane,*poly2->vertices.Get(l+2));

#define CONVEX_TOLER	0.00001 // BSPTOLER 보다 작아야 할듯. ㅜ.ㅡ


							if(fDetHead>-CONVEX_TOLER)
							{
								normal=CrossProduct(*poly2->vertices.Get(l)-*poly2->vertices.Get(l-1),dpoint(poly->plane.a,poly->plane.b,poly->plane.c));
								normal.Normalize();
								DPlaneFromPointNormal(&testplane,*poly2->vertices.Get(l),normal);
								double fDetTail=DPlaneDotCoord(testplane,*poly->vertices.Get(j+2));

								if(fDetTail>-CONVEX_TOLER)		// 이경우 합치면 볼록다각형이 된다 !
								{
									int nInsertPos=j;

									// 직선이면 가운데에 있는 버텍스를 삭제해도 되나, 이경우 edge 가 사라져서 못가는 경우가 발생..
									// 2003.12.1 그러나 지금은 path finding 을 생각하지 않으므로 합친다
									if(fDetHead<CONVEX_TOLER)
									{
										delete *poly->vertices[nInsertPos];
										poly->vertices.erase(poly->vertices.begin()+nInsertPos);
									}
									else
										nInsertPos++;

									for(size_t iv=l+2;iv<l+poly2->vertices.size();iv++)
									{
										poly->vertices.insert(poly->vertices.begin()+nInsertPos,new dpoint(*poly2->vertices.Get(iv)));
										poly->normals.insert(poly->normals.begin()+nInsertPos,new dpoint(*poly2->normals.Get(iv)));
										nInsertPos++;
									}

									// 위와 마찬가지이유로...
									if(fDetTail<CONVEX_TOLER)
									{
										if(nInsertPos==poly->vertices.size())
											nInsertPos=0;
										delete *poly->vertices[nInsertPos];
										poly->vertices.erase(poly->vertices.begin()+nInsertPos);
									}

									poly->RefIDs.insert(poly->RefIDs.end(),poly2->RefIDs.begin(),poly2->RefIDs.end());
//									poly->faces.insert(poly->faces.end(),poly2->faces.begin(),poly2->faces.end());
									poly->fArea+=poly2->fArea;
									poly->plane=plane;
									
									delete poly2;
									erase(begin()+k);

									bMerged=true;
									nMergeCount++;
								}
							}
						}
						if(bMerged) break;
					}
					if(bMerged) break;
				}
				if(!bMerged) k++;
			}
			if(!bMerged) j++;
		}
		i++;
	}

//	log("Merge to convex %d times\n",nMergeCount);
}
*/

/*
void RSBspExporter::ConstructConvexPolygon()
{
	// 일단 볼록다각형으로 합칠수 있는데까지 합쳐본다..

	int i,j,nStartIndex=0,nCount=face.GetCount();
	bool *isdone=new bool[face.GetCount()];
	for(i=0;i<face.GetCount();i++)
		isdone[i]=false;

	for(i=0;i<nCount;i++)
	{
		if(!isdone[i])	
		{
			// 이 폴리곤이 아직 추가안되었으면 같은 평면에 있는 같은 material의 폴리곤들을 집어넣는다.
			rpolygon *f=face.Get(i);
			//_ASSERT(f->nCount==3);

			dplane plane=dplane(f->normal.x,f->normal.y,f->normal.z,f->d);

			for(j=i;j<nCount;j++)
			{
				rpolygon *f2=face.Get(j);
				if(BSPEQ3(f2->normal,f->normal) && BSPEQ(f2->d,f->d) && f->nMaterial==f2->nMaterial)
				{
					RConvexPolygon *pPolygon=new RConvexPolygon;
					for(int k=0;k<f2->nCount;k++)
					{
						pPolygon->vertices.push_back(new dpoint(f2->v[k].coord));
						pPolygon->normals.push_back(new dpoint(f2->v[k].normal));
					}
					
					pPolygon->fArea=0;
					for(k=0;k<f2->nCount-2;k++)
						pPolygon->fArea+=GetArea(f2->v[0].coord,f2->v[k+1].coord,f2->v[k+2].coord);

					if(DPlaneDotNormal(plane,f2->normal)>0)
						pPolygon->plane=plane;
					else
						pPolygon->plane=-plane;
					pPolygon->nMaterial=f->nMaterial;
					pPolygon->dwFlags=f->dwFlags;
					pPolygon->RefIDs.push_back(j);
					//pPolygon->faces.push_back(f2);
					m_ConvexPolygons.push_back(pPolygon);
					isdone[j]=true;
				}
			}

			// 같은 평면의 폴리곤을 다 집어 넣었으면 그 안에서 볼록다각형이 될만한것들을 찾아서 합쳐준다.
			m_ConvexPolygons.MergePolygons(nStartIndex);
			nStartIndex=m_ConvexPolygons.size();
		}
	}
}
*/

bool RSBspExporter::SaveSource(FILE *file)
{
	// csg union & 폴리곤을 다각형으로 합칠만큼 합치고 저장한다.
	RSPolygonList pl;

	log("CSG union - %d objects\n",m_CSGObjects.size());
	CSGUnion(&pl, &faceCol, m_CSGObjects);

/*
	if (!m_NavObjects.empty())
	{
		log("Navi - %d objects\n", m_NavObjects.size());
		CSGUnion(&pl, &faceNavigation, m_NavObjects);
	}
*/

	log("Generating Convex polygons...\n");

	MergePolygons(&pl,&faceOc);
	log("Optimize Polygon %d -> %d \n",pl.GetCount(),faceOc.GetCount());
	pl.DeleteAll();

	int i,j;

	// 폴리곤의 수
	int nConvexPolygon=faceOc.GetCount(),nTotalVertices=0;
	fwrite(&nConvexPolygon,sizeof(int),1,file);

	// 총 버텍스 숫자를 센다.
	for(i=0;i<nConvexPolygon;i++)		
	{
		nTotalVertices+=faceOc.Get(i)->nCount;
	}
	fwrite(&nTotalVertices,sizeof(int),1,file);


	for(i=0;i<nConvexPolygon;i++)
	{
		ip->ProgressUpdate(i*100/nConvexPolygon);
		if(ip->GetCancel()) return false;

		rpolygon *poly=faceOc.Get(i);
		fwrite(&poly->nMaterial,sizeof(int),1,file);
		fwrite(&poly->dwFlags,sizeof(DWORD),1,file);
		poly->GetPlane().SaveFloat(file);
		
		float fArea=(float)poly->GetArea();
		fwrite(&fArea,sizeof(float),1,file);
		fwrite(&poly->nCount,sizeof(int),1,file);
		for(j=0;j<poly->nCount;j++)
			poly->coord(j)->SaveFloat(file);
		for(j=0;j<poly->nCount;j++)
			poly->GetVertex(j)->normal.SaveFloat(file);
		poly->nSourceIndex=i;

//		_ASSERT(i!=182);
	}

	/*
	ConstructConvexPolygon();

	int i,j;

	int nConvexPolygon=m_ConvexPolygons.size(),nTotalVertices=0;
	fwrite(&nConvexPolygon,sizeof(int),1,file);

	for(i=0;i<nConvexPolygon;i++)		// 버텍스 숫자를 센다.
	{
		nTotalVertices+=m_ConvexPolygons[i]->vertices.size();
	}
	fwrite(&nTotalVertices,sizeof(int),1,file);

	for(i=0;i<nConvexPolygon;i++)
	{
		ip->ProgressUpdate(i*100/nConvexPolygon);
		if(ip->GetCancel()) return false;

		RConvexPolygon *poly=m_ConvexPolygons[i];
		fwrite(&poly->nMaterial,sizeof(int),1,file);
		fwrite(&poly->dwFlags,sizeof(DWORD),1,file);
		poly->plane.SaveFloat(file);
//		fwrite(&poly->plane,sizeof(dplane),1,file);
		float fArea=(float)poly->fArea;
		fwrite(&fArea,sizeof(float),1,file);

		int nVertCount=poly->vertices.size();
		fwrite(&nVertCount,sizeof(int),1,file);

		for(j=0;j<nVertCount;j++)
			poly->vertices[j]->SaveFloat(file);
		for(j=0;j<nVertCount;j++)
			poly->normals[j]->SaveFloat(file);

		// 그리고 쪼개질 폴리곤들의 source index 들을 정리한다..
		list<int>::iterator k=poly->RefIDs.begin();
		while(k!=poly->RefIDs.end())
		{
			face.Get(*k)->nSourceIndex=i;
			k++;
		}
	}
	*/

	return true;
}

void GetUVTransformMatrix(D3DXMATRIX *pOut,rvertex *v1,rvertex *v2,rvertex *v3,dpoint *normal=NULL)
{
	dpoint cj=v1->coord;
	dpoint e1=v2->coord-cj;
	dpoint e2=v3->coord-cj;

	if(!normal) {
		// 물론 normal 이 안주어지면 v1,v2,v3로 구할수 있다.
		// 지금은 필요없으므로 코딩하지 않았다.
		_ASSERT(FALSE);
	}

	// j 점을 중심으로 translation
	D3DXMATRIX tr;
	D3DXMatrixTranslation(&tr,-cj.x,-cj.y,-cj.z);

	// e1,e2 (j점을 낀 두 edge)와 삼각형의 법선 벡터 3축을 중심으로 transform
	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	m._11=e1.x;m._12=e1.y;m._13=e1.z;
	m._21=e2.x;m._22=e2.y;m._23=e2.z;
	m._31=normal->x;m._32=normal->y;m._33=normal->z;
	D3DXMatrixInverse(&m,NULL,&m);

	// 위의 두 변환을 거치면 e1,e2좌표계로 변환된다. 이제 uv 좌표로 변환
	D3DXMATRIX m2;
	D3DXMatrixIdentity(&m2);

	m2._11=v2->u-v1->u;
	m2._12=v2->v-v1->v;

	m2._21=v3->u-v1->u;
	m2._22=v3->v-v1->v;

	m2._33=0;

	m2._41=v1->u;
	m2._42=v1->v;

	// 최종변환행렬
	*pOut=tr*m*m2;
}

void RSBspExporter::MergePolygons(RSPolygonList *pSourcePolygons,int nStartIndex)
{
	if(pSourcePolygons->GetCount()<=nStartIndex) return;

	dplane plane=pSourcePolygons->Get(nStartIndex)->GetPlane();

	int nMergeCount=0;

	int i=nStartIndex,j,k,l;

	while(i<pSourcePolygons->GetCount())
	{
		rpolygon *poly=pSourcePolygons->Get(i);

		rvertex *edgea,*edgeb;

		j=0;
		while(j<poly->nCount)
		{
			bool bMerged=false;

			edgea=poly->GetVertex(j);
			edgeb=poly->GetVertex(j+1);

			for(k=i+1;k<pSourcePolygons->GetCount();)
			{
				rpolygon *poly2=pSourcePolygons->Get(k);

				// TODO : normal 이 같은지 uv 가 같은지 확인한다
				if(poly->dwFlags==poly2->dwFlags)				// 플래그가 같아야 합쳐진다 (nopath..cast/receive shadow 등등)
				{
					for(l=0;l<poly2->nCount;l++)
					{
						rvertex *edge2a,*edge2b;
						edge2a=poly2->GetVertex(l);
						edge2b=poly2->GetVertex(l+1);

						/*


										j-1        j l+1       l+2
						 				+-----------+-----------+
										|			|			|
										|	poly	|	poly2	|
										+-----------+-----------+
								    	j+2      j+1 l         l-1

										j,j+1 edge 가 l+1,l edge와 맞을경우에 합친다
						 */


						if(BSPEQ3(edge2a->coord,edgeb->coord) && BSPEQ3(edge2b->coord,edgea->coord) &&
							BSPEQ(edge2a->u,edgeb->u) && BSPEQ(edge2a->v,edgeb->v) &&
							BSPEQ(edge2b->u,edgea->u) && BSPEQ(edge2b->v,edgea->v))				// 맞는 edge 를 찾았다 !
						{

							// j , j-1 , j+1 을 이루는 삼각형에서 uv transform matrix 를 추정한다.

							D3DXMATRIX m;
							GetUVTransformMatrix(&m,poly->GetVertex(j),poly->GetVertex(j-1),poly->GetVertex(j+1),&poly->normal);
							
							dpoint testdp=*poly2->coord(l+2);
							rvector test=rvector(testdp.x,testdp.y,testdp.z);
							D3DXVec3TransformCoord(&test,&test,&m);

							// poly 의 uvtransform matrix 로도 poly2 의 l+2 버텍스와 맞는지 확인해본다
							
							if(fabs(poly2->GetVertex(l+2)->u-test.x)>0.01 ||
								fabs(poly2->GetVertex(l+2)->v-test.y)>0.01 )
								continue;

							/*
							// normal 이 같은지 확인
							dpoint n1=*poly->normals.Get(j+1);
							dpoint n2=*poly2->normals.Get(l);
							_ASSERT(BSPEQ3(n1,n2));
							n1=*poly->normals.Get(j);
							n2=*poly2->normals.Get(l+1);
							_ASSERT(BSPEQ3(n1,n2));
							*/

							dpoint normal;
							dplane testplane;
							normal=CrossProduct(*poly->coord(j)-*poly->coord(j-1),poly->normal);
							normal.Normalize();
							DPlaneFromPointNormal(&testplane,*poly->coord(j),normal);
							double fDetHead=DPlaneDotCoord(testplane,*poly2->coord(l+2));

#define CONVEX_TOLER	0.00001 // BSPTOLER 보다 작아야 할듯. ㅜ.ㅡ


							if(fDetHead>-CONVEX_TOLER)
							{
								normal=CrossProduct(*poly2->coord(l)-*poly2->coord(l-1),poly->normal);
								normal.Normalize();
								DPlaneFromPointNormal(&testplane,*poly2->coord(l),normal);
								double fDetTail=DPlaneDotCoord(testplane,*poly->coord(j+2));

								if(fDetTail>-CONVEX_TOLER)		// 이경우 합치면 볼록다각형이 된다 !
								{
									/*// for debug
									log("Merge %d :: \n",nMergeCount);
									{
									char buffer[256],temp[256];

									RVertexList::iterator dumpi=poly->vertices.begin();
									buffer[0]=0;
									while(dumpi!=poly->vertices.end())
									{
									sprintf(temp," (%3.0f,%3.0f) ",(*dumpi)->x,(*dumpi)->y);
									strcat(buffer,temp);
									dumpi++;
									}
									log("poly1 : %s\n",buffer);

									dumpi=poly2->vertices.begin();
									buffer[0]=0;
									while(dumpi!=poly2->vertices.end())
									{
									sprintf(temp," (%3.0f,%3.0f) ",(*dumpi)->x,(*dumpi)->y);
									strcat(buffer,temp);
									dumpi++;
									}
									log("poly2 : %s\n",buffer);
									}//*/

									// 직선이면 가운데에 있는 버텍스를 삭제해도 되나, 이경우 edge 가 사라져서 못가는 경우가 발생..
									// 2003.12.1 그러나 지금은 path finding 을 생각하지 않으므로 합친다

									int nnewvertices=poly->nCount+poly2->nCount-2;
									
									// 단 uv 가 맞는지는 확인해야 한다

									bool bDelTail = fDetTail<CONVEX_TOLER;
									bool bDelHead = fDetHead<CONVEX_TOLER;

									if(bDelHead) nnewvertices--;
									if(bDelTail) nnewvertices--;

									_ASSERT(nnewvertices>2);

									rvertex *pnewvertices=new rvertex[nnewvertices];

									int iv;
									int nCurPos=0;

									for(iv=l+2;iv<l+poly2->nCount;iv++)
									{
										memcpy(&pnewvertices[nCurPos],poly2->GetVertex(iv),sizeof(rvertex));
										nCurPos++;
									}

									// 직선이되어서 버텍스를 생략해도 괜찮으면 ?  : 그렇지 않으면

									int startj=bDelTail ? j+2 : j+1;
									int endj=bDelHead  ? j+poly->nCount : j+poly->nCount+1;

									for(iv=startj;iv<endj;iv++)
									{
										memcpy(&pnewvertices[nCurPos],poly->GetVertex(iv),sizeof(rvertex));
										nCurPos++;
									}
									_ASSERT(nCurPos==nnewvertices);

									/*
									// 삽입할 위치 앞쪽을 복사하고
									int nInsertPos = (fDetHead<CONVEX_TOLER) ? j : j+1;
									memcpy(pnewvertices,poly->v,sizeof(rvertex)*nInsertPos);

									int nCurPos=nInsertPos;
									// 삽입되어지는 내용을 복사하고
									for(int iv=l+2;iv<l+poly2->nCount;iv++)
									{
										memcpy(&pnewvertices[nCurPos],poly2->GetVertex(iv),sizeof(rvertex));
										nCurPos++;
									}

									// 삽입되어질 위치 뒤쪽을 복사
									nInsertPos = (fDetTail<CONVEX_TOLER) ? j+2 : j+1;
									memcpy(&pnewvertices[nCurPos],&poly->v[nInsertPos],sizeof(rvertex)*(poly->nCount-nInsertPos));
									*/

									delete poly->v;
									poly->nCount=nnewvertices;
									poly->v=pnewvertices;

									/*//for debug
									{
									char buffer[256],temp[256];

									RVertexList::iterator dumpi=poly->vertices.begin();
									buffer[0]=0;
									while(dumpi!=poly->vertices.end())
									{
									sprintf(temp," (%3.0f,%3.0f) ",(*dumpi)->x,(*dumpi)->y);
									strcat(buffer,temp);
									dumpi++;
									}
									log("poly merged : %s\n",buffer);
									}//*/

									poly->normal=dpoint(plane.a,plane.b,plane.c);
									poly->d=plane.d;

									pSourcePolygons->Delete(k);

									bMerged=true;
									nMergeCount++;
								}
							}
						}
						if(bMerged) break;
					}
					if(bMerged) break;
				}
				if(!bMerged) k++;
			}
			if(!bMerged) j++;
		}
		i++;
	}
}

void RSBspExporter::MergePolygons(RSPolygonList *pSourcePolygons,RSPolygonList *pOutputPolygons)
{
	// 일단 볼록다각형으로 합칠수 있는데까지 합쳐본다..

	int i,j,nStartIndex=0,nCount=pSourcePolygons->GetCount();
	bool *isdone=new bool[nCount];
	for(i=0;i<nCount;i++)
		isdone[i]=false;

	for(i=0;i<nCount;i++)
	{
		if(!isdone[i])	
		{
			// 이 폴리곤이 아직 추가안되었으면 같은 평면에 있는 같은 material의 폴리곤들을 집어넣는다.
			rpolygon *f=pSourcePolygons->Get(i);

			dplane plane=f->GetPlane();

			// 분명히 f 의 버텍스들은 plane 위에 있어야 한다. 그렇지 않으면 뻗는다.

			for(j=i;j<nCount;j++)
			{
				rpolygon *f2=pSourcePolygons->Get(j);
//				_ASSERT(f2->nID!=182);
				if(f->nMaterial==f2->nMaterial && DotProduct(f->normal,f2->normal)>0.9f)
				{
					bool bCoplanar=true;
					for(int k=0;k<f2->nCount;k++)
					{
						if(signof(DPlaneDotCoord(plane,f2->v[k].coord))!=ZERO) {
							bCoplanar=false;
							break;
						}
					}
					if(!bCoplanar) continue;

					pOutputPolygons->Add(new rpolygon(f2));

					isdone[j]=true;
				}
			}

			// 같은 평면의 폴리곤을 다 집어 넣었으면 그 안에서 볼록다각형이 될만한것들을 찾아서 합쳐준다.
			MergePolygons(pOutputPolygons,nStartIndex);
			nStartIndex=pOutputPolygons->GetCount();
		}
	}

	delete isdone;
}

