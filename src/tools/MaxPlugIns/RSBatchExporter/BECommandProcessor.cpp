#include "BECommandProcessor.h"
#include "BEParser.h"
#include "BELog.h"
#include "exporter.h"
#include "fileinfo.h"
#include "RSMaterialList.h"

RSMaterialList *pML=NULL;
char destdir[256],sourcedir[256],rmlfile[256];

void ProcessCommand(Interface *ip,BECommand *command)
{
	switch(command->nCommand)
	{
	case CRMLFILE :
		{
			char buffer[256];
			if(destdir[0] && (!IsFullPath(command->szBuffer)))
			{
				strcpy(buffer,destdir);
				strcat(buffer,command->szBuffer);
			}
			else
				strcpy(buffer,command->szBuffer);
				
			log("RML File set to %s\n",buffer);
			if(pML)
			{
				pML->Save(rmlfile);
				delete pML;
			}
			pML=new RSMaterialList;
			strcpy(rmlfile,buffer);
		}
		break;

	case CSOURCE :
		{
			log("Source directory was set to %s\n",command->szBuffer);
			strcpy(sourcedir,command->szBuffer);
			if(sourcedir[strlen(sourcedir)-1]!='\\') strcat(sourcedir,"\\");
		}
		break;

	case CDESTINATION :
		{
			log("Destination directory was set to %s\n",command->szBuffer);
			strcpy(destdir,command->szBuffer);
			if(destdir[strlen(destdir)-1]!='\\') strcat(destdir,"\\");
		}
		break;

	case CEXPORT :
		{
			char szDir[256], szDrive[4], szFileName[256], szExt[16];
			_splitpath(command->szBuffer, szDrive, szDir, szFileName, szExt);
			char filename[256];
			int i;
			
			strcpy(filename,command->szBuffer);
			if(!IsExist(command->szBuffer))
			{
				strcpy(filename,sourcedir);
				strcat(filename,szFileName);
				strcat(filename,szExt);
				if(!IsExist(filename))
				{
					log("     File not found %s\n",command->szBuffer);
					break;
				}
			}

			if(!ip->LoadFromFile(filename))
			{
				log("     File Open Failed %s\n",filename);
				break;
			}
			ip->ForceCompleteRedraw();
			log("Processing %s\n",filename);
			Exporter exporter;
			RSMObject *rsm=new RSMObject;
			exporter.DoExport("test.rsm",ip,rsm);
			bool bNeedToLoose=false;
			for(i=0;i<command->AnimationList.GetCount();i++)
			{
				AnimationParam *ap=command->AnimationList.Get(i);
				if(ap->iAnimationType==AM_VERTEX)
					bNeedToLoose|=true;
			}
			rsm->Optimize(bNeedToLoose);
			for(i=0;i<command->AnimationList.GetCount();i++)
			{
				AnimationParam *ap=command->AnimationList.Get(i);
				log("     Add Animation %s using %s \n",ap->szAnimationName,ap->szMaxFileName);

				char szDir[256], szDrive[4], szFileName[256], szExt[16];
				_splitpath(ap->szMaxFileName, szDrive, szDir, szFileName, szExt);
				char filename[256];
				
				strcpy(filename,ap->szMaxFileName);
				if(!IsExist(ap->szMaxFileName))
				{
					strcpy(filename,sourcedir);
					strcat(filename,szFileName);
					strcat(filename,szExt);
					if(!IsExist(filename))
					{
						log("     Error : File not found %s\n",ap->szMaxFileName);
						break;
					}
				}

				if(!ip->LoadFromFile(filename))
				{
					log("     Error : File Open Failed %s\n",filename);
					break;
				}

				AnimConstructor *ac=new AnimConstructor;
				ac->BuildAnimation(ip,ap->szAnimationName,ap->fAnimationSpeed,ap->iAnimationType);
				if(!rsm->AddAnimation(ac))
				{
					log("     Error : Animation Does not match.\n");
					delete ac;
				}
			}
			if(destdir[0])
			{
				char szDir[256], szDrive[4], szFileName[256], szExt[16];
				_splitpath(filename, szDrive, szDir, szFileName, szExt);

				char tempname[256];
				if(!IsFullPath(command->szBuffer2))
				{
					strcpy(tempname,destdir);
					strcat(tempname,command->szBuffer2);
				}
				else strcpy(tempname,command->szBuffer2);
				rsm->SaveRSM(tempname);
				if(pML)
					rsm->AddMaterials(pML);
				else
				{
					char tempname2[256];
					ReplaceExtension(tempname2,tempname,"rml");
					rsm->SaveRML(tempname2);
				}
			}
			else
			{
				char szDir[256], szDrive[4], szFileName[256], szExt[16];
				_splitpath(filename, szDrive, szDir, szFileName, szExt);

				char tempname[256];
				if(!IsFullPath(command->szBuffer2))
				{
					strcpy(tempname,szDrive);
					strcat(tempname,szDir);
					strcat(tempname,command->szBuffer2);
				}
				else strcpy(tempname,command->szBuffer2);
				rsm->SaveRSM(tempname);
				if(pML)
					rsm->AddMaterials(pML);
				else
				{
					char tempname2[256];
					ReplaceExtension(tempname2,tempname,"rml");
					rsm->SaveRML(tempname2);
				}
			}
			delete rsm;
		}
		break;
	}
}

void DoBatchExport(Interface *ip,HWND hWnd,BECommandList *pCommandList)
{
	pML=NULL;
	sourcedir[0]=0;
	destdir[0]=0;
	rmlfile[0]=0;

	for(int i=0;i<pCommandList->GetCount();i++)
	{
		BECommand *command=pCommandList->Get(i);
		ProcessCommand(ip,command);
	}
	if(pML)
	{
		pML->Save(rmlfile);
		delete pML;
	}
}