#include "stdafx.h"
#include "MFont.h"
#include "MDrawContext.h"

// MFont Implementation
///////////////////////
MFont::MFont(void)
{
#ifdef _DEBUG
	m_nTypeID = MINT_BASE_CLASS_TYPE;
#endif
	m_szName[0] = 0;

	m_nOutlineStyle = 0;
	m_ColorArg1 = 0;
	m_ColorArg2 = 0;
}

MFont::~MFont(void)
{
}

bool MFont::Create(const char* szName)
{
	_ASSERT(strlen(szName)<MFONT_NAME_LENGTH);
	strcpy_safe(m_szName, szName);

	return true;
}

void MFont::Destroy(void)
{
}

int MFont::GetWidthWithoutAmpersand(const char* szText, int nSize)
{
	int nAmpWidth = GetWidth("&");
	int nTextLen = strlen(szText);
	int nAmpCount = 0;
	for(int i=0; i<((nSize>0)?nSize:nTextLen); i++){
		if(szText[i]=='&') nAmpCount++;
	}
	int nTextWidth = GetWidth(szText, nSize);
	return nTextWidth-nAmpWidth*nAmpCount;
}


/*
int MFont::GetPossibleCharacterCount(int nRectWidth, const char* szText)
{
	int nLength=strlen(szText);
	char *text=strdup(szText);

	for(int i=0;i<nLength;i++)
	{
		text[i]=0;
		if(i>0) text[i-1]=szText[i-1];

		char *puretext=MDrawContext::GetPureText(text);
		if(nRectWidth<GetWidth(puretext))
		{
			free(text);
			free(puretext);
			return i;
		}
		free(puretext);
		
		if((unsigned char)szText[i]>127) {
			text[i]=szText[i];
			i++;
		}
	}

	free(text);
	return nLength;
}
*/
