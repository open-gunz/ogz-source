#include "stdafx.h"
#include "MFileBox.h"
#include <io.h>
#include "Shlwapi.h"

MFileBox::MFileBox(const char* szFilter, MWidget* pParent, MListener* pListener)
	: MListBox(szFilter, pParent, pListener)
{
	Refresh(szFilter);
}

MFileBox::~MFileBox(void)
{
}

void MFileBox::Refresh(const char* szFilter)
{
	RemoveAll();
	if(szFilter==NULL) szFilter = m_szFilter;
	else strcpy_safe(m_szFilter, szFilter);

    struct _finddata_t c_file;
    long hFile;
	char szName[_MAX_PATH];
	int len;

	if( (hFile = _findfirst( szFilter, &c_file )) != -1L ){
		do{
			len = strlen(c_file.name);
			//strncpy(szName, c_file.name, len-4);
			//szName[len-4]='\0';
			strcpy_safe(szName, c_file.name);
			//strupr(szName);
			Add(szName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}
	
	Sort();

	PathSearchAndQualify(m_szFilter,m_szBaseDir,sizeof(m_szBaseDir));
	PathRemoveFileSpec(m_szBaseDir);
}

const char* MFileBox::GetBaseDir(void)
{
	return m_szBaseDir;
}
