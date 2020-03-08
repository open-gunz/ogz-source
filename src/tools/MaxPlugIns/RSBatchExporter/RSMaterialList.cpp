// RSMaterialList.cpp: implementation of the RSMaterialList class.
// 1999.1.4 by dubble
//////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "RSMaterialList.h"
#include "FileInfo.h"
#include "dib.h"
#include "image24.h"

RSTexture::RSTexture()
{
	bLocked=FALSE;isInDisk=FALSE;
	Name=NULL;Data=NULL;
	RefferenceCount=0;
}

RSTexture::~RSTexture()
{
	if(Name) delete []Name;
	if(Data) delete []Data;
}

RSTexture::CreateFromBMP(const char *filename)
{

	CDib dib;
	CImage24 image;
	if(dib.Open(NULL,filename))
	{
		if(image.Open(&dib))
		{
			char fname[_MAX_FNAME];
			GetPureFilename(fname,filename);
			if(Name) delete []Name;
			Name=new char[strlen(fname)+1];
			strcpy(Name,fname);
			if(Data) delete []Data;
			Data=new char[image.GetDataSize()];
			memcpy(Data,image.GetData(),image.GetDataSize());
			x=image.GetWidth();
			y=image.GetHeight();
			bLocked=TRUE;isInDisk=FALSE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
	return TRUE;
}

void RSTexture::CreateFromTexture(const RSTexture *t)
{
	if(Name) delete []Name;
	if(Data) delete []Data;
	x=t->x;y=t->y;
	Name=new char[strlen(t->Name)+1];strcpy(Name,t->Name);
	Data=new char[x*y*3];memcpy(Data,t->Data,x*y*3);
	isInDisk=false;
}

DWORD RSTexture::GetPixel(int x,int y)
{
	unsigned char a,r,g,b;
	a=0;//(unsigned char)Data[RSTexture::x*y+x];
	r=(unsigned char)Data[(RSTexture::x*y+x)*3];
	g=(unsigned char)Data[(RSTexture::x*y+x)*3+1];
	b=(unsigned char)Data[(RSTexture::x*y+x)*3+2];
	return (DWORD)a << 24 | (DWORD)r << 16 | (DWORD)g << 8 | (DWORD)b;
}

RSMaterial::RSMaterial()
{
	Name=NULL;TextureIndices=NULL;TextureHandles=NULL;TextureHandle=0;
	Ambient=0;Diffuse=0xffffff;Colorkey=0;
	ShadeMode=RSSHADEMODE_NORMAL;b2Sided=0;bColorkey=0;bAnimation=0;nTexture=0;
	AnimationSpeed=10;bLocked=FALSE;
}

RSMaterial::~RSMaterial()
{
	if(Name)delete[]Name;
	if(TextureIndices) delete[]TextureIndices;
	if(TextureHandles) delete[]TextureHandles;
}

RSMaterialList::RSMaterialList()
{
	pFileName=NULL;
}

RSMaterialList::~RSMaterialList()
{
/*	// for debug
	OutputDebugString("Texture Ref. Count :");
	for(int i=0;i<m_Textures.GetCount();i++)
	{
		RSTexture *t=m_Textures.Get(i);
		char buf[256];
		sprintf(buf,"%3d",t->RefferenceCount);
		OutputDebugString(buf);
	}
	OutputDebugString("\n");
	// end of debug */
	if(pFileName) delete []pFileName;
}

void RSMaterialList::SetName(const char *name)
{
	if(pFileName) delete []pFileName;
	pFileName=new char[strlen(name)+1];
	strcpy(pFileName,name);
}

void RSMaterialList::Close()
{
	if(pFileName) delete []pFileName;
	pFileName=NULL;
	m_Materials.DeleteAll();
	m_Textures.DeleteAll();
}

void RSMaterialList::Add(RSMaterial *m)
{
	m_Materials.Add(m);
}

int RSMaterialList::AddTexture(RSTexture *t)
{
	m_Textures.Add(t);
	return m_Textures.GetCount()-1;
}

RSMaterial* RSMaterialList::Get(int i)
{
	if((i<0)||(i>=m_Materials.GetCount())) return NULL;
	return m_Materials.Get(i);
}

RSTexture* RSMaterialList::GetTexture(int i)
{
	if((i<0)||(i>=m_Textures.GetCount())) return NULL;
	return m_Textures.Get(i);
}

int RSMaterialList::GetTextureIndex(const char *pAliasName)
{
	for(int i=0; i<m_Textures.GetCount(); i++){
		if(strcmp(m_Textures.Get(i)->Name, pAliasName) == 0)
			return i;
	}
	return -1;
}

RSTexture* RSMaterialList::GetTexture(const char *pAliasName)
{
	int i=GetTextureIndex(pAliasName);
	if(i==-1) return NULL;
		else return m_Textures.Get(i);
}

int RSMaterialList::GetIndex(const char *pAliasName)
{
	for(int i=0; i<m_Materials.GetCount(); i++){
		if(strcmp(m_Materials.Get(i)->Name, pAliasName) == 0)
			return i;
	}
	return -1;
}

RSMaterial* RSMaterialList::Get(const char *pAliasName)
{
	int i=GetIndex(pAliasName);
	if(i==-1) return NULL;
		else return m_Materials.Get(i);
}

BOOL RSMaterialList::IsExist(const char *pAliasName)
{
	for(int i=0; i<m_Materials.GetCount(); i++){
		if(strcmp(m_Materials.Get(i)->Name, pAliasName) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL RSMaterialList::IsExistTexture(const char *pAliasName)
{
	for(int i=0; i<m_Textures.GetCount(); i++){
		if(strcmp(m_Textures.Get(i)->Name, pAliasName) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL RSMaterialList::Save(const char *pFileName)
{
	int nError=0;
#define TEMP_FILE_NAME "temp~.rml"

	FILE *file;
	file=fopen(TEMP_FILE_NAME,"wb+");
	if(!file) return false;

	int ret=Save(file);
	fclose(file);

	if(!DeleteFile(pFileName)) 
		nError=GetLastError();
	if(nError==0 || nError==2)		// ok or File not found.
	{
		if(MoveFile(TEMP_FILE_NAME,pFileName)) 
		{
			SetName(pFileName);
			Close();
			Open(pFileName);
			return true;
		}
	}
	else
		DeleteFile(TEMP_FILE_NAME);
	return false;
}

bool RSMaterialList::Save(FILE *file)
{
	Sort();

	int i;
	bool ErrorOccured=false;

	// Header
	RMLHEADER RmlHdr;
	RmlHdr.nFileID = MATERIAL_BLAST2_FILE_ID;
	RmlHdr.nFileBuild = MATERIAL_BLAST2_FILE_BUILD;
	RmlHdr.nMaterialCount = m_Materials.GetCount();
	RmlHdr.nTextureCount = m_Textures.GetCount();

	if(!fwrite(&RmlHdr,sizeof(RMLHEADER),1,file)){
		return false;
	}

	for(i=0;i<m_Materials.GetCount();i++)
	{
		RSMaterial *m=m_Materials.Get(i);
		ErrorOccured|=!fwrite(m->Name,strlen(m->Name)+1,1,file);
		ErrorOccured|=!fwrite(&m->Diffuse,sizeof(DWORD),1,file);
		ErrorOccured|=!fwrite(&m->Ambient,sizeof(DWORD),1,file);
		ErrorOccured|=!fwrite(&m->Colorkey,sizeof(DWORD),1,file);
		ErrorOccured|=!fwrite(&m->ShadeMode,sizeof(RSSHADEMODE),1,file);
		ErrorOccured|=!fwrite(&m->b2Sided,sizeof(BOOL),1,file);
		ErrorOccured|=!fwrite(&m->bColorkey,sizeof(BOOL),1,file);
		ErrorOccured|=!fwrite(&m->AnimationSpeed,sizeof(DWORD),1,file);
		ErrorOccured|=!fwrite(&m->nTexture,sizeof(DWORD),1,file);
		if(m->nTexture)
		ErrorOccured|=!fwrite(m->TextureIndices,sizeof(DWORD)*m->nTexture,1,file);
		if(ErrorOccured) goto Error;
	}

	for(i=0;i<m_Textures.GetCount();i++)
	{
		RSTexture *t=m_Textures.Get(i);
		ErrorOccured|=!fwrite(t->Name,strlen(t->Name)+1,1,file);
		ErrorOccured|=!fwrite(&t->x,sizeof(int),1,file);
		ErrorOccured|=!fwrite(&t->y,sizeof(int),1,file);
		int nByte=(t->x)*(t->y)*3;
		Lock(i);
		ErrorOccured|=!fwrite(t->Data,nByte,1,file);
		Unlock(i);
		if(ErrorOccured) goto Error;
	}

Error:
	return !ErrorOccured;
}

void ReadString(FILE *file, char ** name)
{
	char buf[256];
	int	i=0;
	do
	{
		fread(&buf[i++],1,1,file);
	} while(buf[i-1]);
	*name=new char[i];
	strcpy(*name,buf);
}

BOOL RSMaterialList::Open(const char *pFileName)
{
	FILE *file;
	file=fopen(pFileName,"rb");
	if(!file) return false;

	bool bOk=Open(file);
	if(bOk)
		SetName(pFileName);
	fclose(file);
	return bOk;
}

bool RSMaterialList::Open(FILE *file)
{
	bool ErrorOccured=false;

	// Header
	RMLHEADER RmlHdr;
	if(fread( &RmlHdr, sizeof(RMLHEADER), 1, file ) == FALSE){
		return false;
	}

	if((RmlHdr.nFileID != MATERIAL_BLAST2_FILE_ID)||
		(RmlHdr.nFileBuild != MATERIAL_BLAST2_FILE_BUILD)) return false;

	for(int i=0;i<RmlHdr.nMaterialCount;i++)
	{
		RSMaterial *m=new RSMaterial;
		ReadString(file,&m->Name);
		ErrorOccured|=!fread(&m->Diffuse,sizeof(DWORD),1,file);
		ErrorOccured|=!fread(&m->Ambient,sizeof(DWORD),1,file);
		ErrorOccured|=!fread(&m->Colorkey,sizeof(DWORD),1,file);
		ErrorOccured|=!fread(&m->ShadeMode,sizeof(RSSHADEMODE),1,file);
		ErrorOccured|=!fread(&m->b2Sided,sizeof(BOOL),1,file);
		ErrorOccured|=!fread(&m->bColorkey,sizeof(BOOL),1,file);
		ErrorOccured|=!fread(&m->AnimationSpeed,sizeof(DWORD),1,file);
		ErrorOccured|=!fread(&m->nTexture,sizeof(DWORD),1,file);
		if(m->nTexture)
		{
		m->TextureIndices=new int[m->nTexture];
		ErrorOccured|=!fread(m->TextureIndices,sizeof(DWORD),m->nTexture,file);
		}
		m_Materials.Add(m);
		if(ErrorOccured) goto Error;
	}

	for(i=0;i<RmlHdr.nTextureCount;i++)
	{
		RSTexture *t=new RSTexture;
		ReadString(file,&t->Name);
		ErrorOccured|=!fread(&t->x,sizeof(int),1,file);
		ErrorOccured|=!fread(&t->y,sizeof(int),1,file);
		int nByte=(t->x)*(t->y)*3;
		t->bLocked=FALSE;t->isInDisk=TRUE;
		t->FilePosition=ftell(file);
		fseek(file,nByte,SEEK_CUR);
		m_Textures.Add(t);
		if(ErrorOccured) goto Error;
	}

Error:
	return !ErrorOccured;
}

BOOL RSMaterialList::Lock(int i)
{
	RSTexture *t=GetTexture(i);
	t->RefferenceCount++;
	if((t->bLocked)||(!t->isInDisk)) return FALSE;


	FILE *file;
	file=fopen(pFileName,"rb");
	if(!file) return false;
	t->Data=new char[t->x*t->y*3];
	fseek(file,t->FilePosition,SEEK_SET);
	if(!fread(t->Data,t->x*t->y*3,1,file))
		return false;
	fclose(file);
	t->bLocked=TRUE;

/*
	HANDLE hFile;
	hFile=CreateFile(pFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) 
		return FALSE;
	t->Data=new char[t->x*t->y*3];
	SetFilePointer(hFile,t->FilePosition,NULL,FILE_BEGIN);
	DWORD nRead;
	if(!ReadFile(hFile,t->Data,t->x*t->y*3,&nRead,NULL)) 
		return FALSE;
	CloseHandle(hFile);
	t->bLocked=TRUE;

*/
	return TRUE;
}

BOOL RSMaterialList::Lock(char *pAliasName)
{
	return Lock(GetTextureIndex(pAliasName));
}

void RSMaterialList::Unlock(int i)
{
	_ASSERT((i>=0)&&(i<=m_Textures.GetCount()));
	RSTexture *t=GetTexture(i);
	if(t->RefferenceCount==0) return;
	t->RefferenceCount--;
//	_ASSERT(t->RefferenceCount>=0);
	if((!t->bLocked)||(!t->isInDisk)||(t->RefferenceCount>0)) return;
	delete []t->Data;
	t->Data=NULL;
	t->bLocked=FALSE;	
}

void RSMaterialList::Unlock(char *pAliasName)
{
	Unlock(GetTextureIndex(pAliasName));
}

BOOL RSMaterialList::LockMaterial(int a)
{
	BOOL Error=FALSE;
	RSMaterial *m=Get(a);
	if(m->bLocked) return FALSE;
	for(int i=0;i<m->nTexture;i++)
		Error|=Lock(m->TextureIndices[i]);
	m->bLocked=TRUE;
	return Error;
}

void RSMaterialList::UnlockMaterial(int a)
{
	RSMaterial *m=Get(a);
	if(!m->bLocked) return;
	for(int i=0;i<m->nTexture;i++)
		Unlock(m->TextureIndices[i]);
	m->bLocked=FALSE;
}

void RSMaterialList::Sort()
{
	int n=m_Textures.GetCount();
	if(n>1)
	{
		for(int i=0;i<n;i++)
			m_Textures.Get(i)->index=i;
		m_Textures.Sort();
		int *changeindex=new int[n];
		for(i=0;i<n;i++)
			changeindex[m_Textures.Get(i)->index]=i;

		for(i=0;i<m_Materials.GetCount();i++)	// correct texture indices
		{
			RSMaterial *m=m_Materials.Get(i);
			for(int j=0;j<m->nTexture;j++)
				m->TextureIndices[j]=changeindex[m->TextureIndices[j]];
		}
		delete []changeindex;
	}
	m_Materials.Sort();

}

void RSMaterialList::Delete(int i)
{
	m_Materials.Delete(i);
}

void RSMaterialList::Delete(const char *pAliasName)
{
	Delete(GetIndex(pAliasName));
}

BOOL RSMaterialList::DeleteTexture(int iTarget)
{
	BOOL isExistDeleted=FALSE;
	m_Textures.Delete(iTarget);					
	// reindexing
	for(int i=0;i<m_Materials.GetCount();i++)
	{
		RSMaterial *m=m_Materials.Get(i);
		int	*temp=new int[m->nTexture],count=0;
		for(int j=0;j<m->nTexture;j++)
		{
			if(m->TextureIndices[j]<iTarget)
				temp[count++]=m->TextureIndices[j];
			else
			if(m->TextureIndices[j]>iTarget)
				temp[count++]=m->TextureIndices[j]-1;
			else
				isExistDeleted=TRUE;
		}
		delete []m->TextureIndices;
		m->nTexture=count;
		m->TextureIndices=count?temp:NULL;
	}
	return isExistDeleted;
}

BOOL RSMaterialList::DeleteTexture(const char *pAliasName)
{
	return DeleteTexture(GetTextureIndex(pAliasName));
}

void RSMaterialList::AddLibrary(RSMaterialList *ml)
{
	int i,cnt;
	RSTexture *pTex;
	RSMaterial *pMat;

	// Add Textures
	cnt = ml->GetTextureCount();
	int *TexIndices=new int[cnt];

	for( i = 0; i < cnt; i ++ ){
		pTex = ml->GetTexture(i);
		if( pTex ){
			if( IsExistTexture( pTex->Name ) == TRUE ){
				TexIndices[i]=GetTextureIndex( pTex->Name );
					ml->Lock(i);
					GetTexture(TexIndices[i])->CreateFromTexture(pTex);
					ml->Unlock(i);
			} else {
				// Add New Texture
				RSTexture *newtex=new RSTexture;
				ml->Lock(i);
				newtex->CreateFromTexture(pTex);
				AddTexture( newtex );
				ml->Unlock(i);
				TexIndices[i]=GetTextureCount()-1;
			}			
		} else _ASSERT(FALSE);
	}
	// Add Materials
	cnt = ml->GetCount();
	for( i = 0; i < cnt; i ++ ){
		pMat = ml->Get( i );
		if( pMat ){			
			RSMaterial *mat=NULL;
			if( IsExist( pMat->Name ) == TRUE ){
				mat = Get( pMat->Name );
			} else {
				mat=new RSMaterial;
				Add(mat);
			}				
			if(mat)
			{
				memcpy(mat,pMat,sizeof(RSMaterial));
				mat->Name=new char[strlen(pMat->Name)+1];strcpy(mat->Name,pMat->Name);
				mat->TextureIndices=new int[mat->nTexture];
				for(int j=0;j<mat->nTexture;j++)
					mat->TextureIndices[j]=TexIndices[pMat->TextureIndices[j]];
			}
		}			
	}

	delete []TexIndices;
}

