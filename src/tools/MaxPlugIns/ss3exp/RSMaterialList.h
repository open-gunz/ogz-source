// RSMaterialList.h: interface for the RSMaterialList class.
// 1999.1.4 by dubble
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSMATERIALLIST_H__B54BFE30_A430_11D2_9A02_00AA006E4A4E__INCLUDED_)
#define AFX_RSMATERIALLIST_H__B54BFE30_A430_11D2_9A02_00AA006E4A4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
//#include "windows.h"
#include "CMlist.h"

#define MATERIAL_BLAST2_FILE_ID	6666
#define MATERIAL_BLAST2_FILE_BUILD 1

#define MATERIAL_NAME_LIMIT_LENGTH	64		// 고정 크기 문자열을 위한 Definition(이전버전과의 호환)

typedef struct _tagRMLHEADER{
	int		nFileID;
	int		nFileBuild;
	int		nMaterialCount;
	int		nTextureCount;
}RMLHEADER, *PRMLHEADER;

class RSTexture
{
public:
	RSTexture();
	virtual ~RSTexture();

	BOOL CreateFromBMP(const char *);
	void CreateFromTexture(const RSTexture *);
	DWORD GetPixel(int x,int y);

	char *Data;
	char *Name;
	int		x,y;

	int	FilePosition,RefferenceCount,index;
	BOOL bLocked,isInDisk;
};

class RSTextureList:public CMLinkedList<RSTexture>
{
	int Compare(RSTexture *pRecord1,RSTexture *pRecord2){
		if(strcmp(pRecord1->Name,pRecord2->Name)>0)return 1;
		else if(strcmp(pRecord1->Name,pRecord2->Name)==0)return 0;
		else return -1;}
};

enum RSSHADEMODE
{
	RSSHADEMODE_NORMAL	= 0,
	RSSHADEMODE_ADD	=	1,
	RSSHADEMODE_ALPHAMAP = 2,
};

class RSMaterial
{
public:
	RSMaterial();
	virtual ~RSMaterial();

	DWORD Diffuse,Ambient,Colorkey,AnimationSpeed,thistime;
	RSSHADEMODE ShadeMode;
	BOOL bColorkey,bAnimation,b2Sided;
	
	int	TextureIndex,nTexture;
	int *TextureIndices;

	char *Name;
	int TextureHandle;
	int *TextureHandles;

	BOOL bLocked;

	// 메터리얼에서 쓰고 있는 텍스쳐 개수 얻기
	int GetTextureCount(void){
		return nTexture;
	}
	// 매터리얼의 텍스쳐 인덱스로 텍스쳐 리스트에서의 인덱스 얻기
	int GetTextureIndex(int i){
		_ASSERT(i>=0 && i<nTexture);
		return TextureIndices[i];
	}

	BOOL isAnimationEnded(){return (nTexture*AnimationSpeed<thistime);}
};

class RSMaterialList  
{
public:

	RSMaterialList();
	virtual ~RSMaterialList();
	class RSMaterials:public CMLinkedList<RSMaterial>{
	public:
	int Compare(RSMaterial *pRecord1,RSMaterial *pRecord2){
		if(strcmp(pRecord1->Name,pRecord2->Name)>0)return 1;
		else if(strcmp(pRecord1->Name,pRecord2->Name)==0)return 0;
		else return -1;}
	} m_Materials;
	RSTextureList m_Textures;

	char *GetName() { return pFileName; }
	void SetName(const char *);
	void Add(RSMaterial *);
	int AddTexture(RSTexture *);
	
	RSTexture* GetTexture(int i);
	int GetTextureIndex(const char *pAliasName);
	RSTexture* GetTexture(const char*pAliasName);

	RSMaterial* Get(int i);
	int GetIndex(const char *pAliasName);
	RSMaterial* Get(const char *pAliasName);

	int GetCount() { return m_Materials.GetCount();};
	int GetTextureCount() { return m_Textures.GetCount();};

	BOOL IsExist(const char *pAliasName);
	BOOL IsExistTexture(const char *pAliasName);
	BOOL Save(const char*);
	bool Save(FILE *file);
	BOOL Open(const char*);
	bool Open(FILE *file);
	void Close();
	BOOL Lock(int i);
	BOOL Lock(char *pAliasName);
	void Unlock(int i);
	void Unlock(char *pAliasName);
	BOOL LockMaterial(int i);
	void UnlockMaterial(int i);
	void Sort();   // be careful that sort will break texture's file position
	void Delete(int i);
	void Delete(const char *pAliasName);
	BOOL DeleteTexture(int i);
	BOOL DeleteTexture(const char *pAliasName);

	void AddLibrary(RSMaterialList *ml);

private:
	char	*pFileName;
	int		m_nFileOffset;
};

#endif // !defined(AFX_RSMATERIALLIST_H__B54BFE30_A430_11D2_9A02_00AA006E4A4E__INCLUDED_)
