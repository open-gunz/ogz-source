#pragma once

#include <map>
#include <string>
#include "MZFileSystem.h"

#define CHAR_MSG_VALUE			'$'

template<class _T>
class MStringRes
{
private:
	std::map<_T, std::string>		m_StringMap;
	std::string				m_strTOK_TAG;
	std::string				m_strTOK_ATTR;
public:
	MStringRes( const char* pszTOK_TAG = "STR", const char* pszTOK_ATTR = "id" )
		: m_strTOK_TAG( pszTOK_TAG ), m_strTOK_ATTR( pszTOK_ATTR )
	{

	}

	virtual ~MStringRes()
	{

	}
	void Release()
	{
		m_StringMap.clear();
	}
	bool Initialize(const char* pszFileName, const int nLangID, MZFileSystem *pfs = 0)
	{
		if (!pszFileName)
			return false;

		MXmlDocument aXml;
		{
			mlog("Load XML from memory : %s(0x%04X) ", pszFileName, nLangID);

			if (!aXml.LoadFromFile(pszFileName, pfs))
			{
				mlog("- FAIL\n");
				return false;
			}

			mlog("- SUCCESS\n");
		}

		MXmlElement aParent = aXml.GetDocumentElement();
		const int	iCount  = aParent.GetChildNodeCount();

		MXmlElement	aChild;
		_T			CID;
		char		szTagName[256];
		char		szContents[512];

		for (int i = 0; i < iCount; i++)
		{
			aChild = aParent.GetChildNode(i);
			aChild.GetTagName(szTagName);

			if (_stricmp( szTagName,m_strTOK_TAG.c_str() )==0 ||
				_stricmp( szTagName, "MSG" )==0 ||
				_stricmp( szTagName, "MESSAGE" )==0)
			{
				if(aChild.GetAttribute(&CID,m_strTOK_ATTR.c_str()))
				{
					_ASSERT( m_StringMap.find(CID)==m_StringMap.end() );

					aChild.GetContents(szContents);

					m_StringMap.emplace(CID, szContents);
				}
			}
		}

		return true;
	}

	template<size_t size>
	bool Translate(char(&poutStr)[size], const _T& code, const int argnum, const char* arg1, va_list args) {
		return Translate(poutStr, size, code, argnum, arg1, args);
	}
	bool Translate(char* poutStr, int maxlen, const _T& code, const int argnum, const char* arg1, va_list args )
	{
		auto itor = m_StringMap.find(code);
		if(itor==m_StringMap.end())
		{
			_ASSERT(0);
			return false;
		}

		if ((argnum <= 0) || (argnum > 9))
		{
			strcpy_safe(poutStr, maxlen, (*itor).second.c_str());
			return true;
		}

		const char* argv[9] = {NULL, };

		argv[0] = arg1;

		for (int i = 1; i < argnum; i++)
		{
			argv[i] = va_arg(args, const char*);
		}

		bool bPercent=false;

		int taridx = 0;
		poutStr[taridx] = 0;
		std::string formatstring = (*itor).second;
		
		for(size_t j=0;j<formatstring.size();j++)
		{
			char cur=formatstring[j];
			if(bPercent)
			{
				if(cur==CHAR_MSG_VALUE)
				{
					poutStr[taridx++] = CHAR_MSG_VALUE;
					poutStr[taridx] = 0;
				}
				else
				if ( ('1' <= cur) && (cur <= '9') )
				{
					int nParam = cur - '0' - 1;
					if ( (nParam < argnum) && (argv[nParam] != NULL) )
					{
						strcat_safe(poutStr, maxlen, argv[nParam]);
						taridx += (int)strlen(argv[nParam]);
					}
					else
					{
						assert(0);	
					}
				}

				bPercent = false;
				continue;
			}

			if(!bPercent)
			{
				if(cur==CHAR_MSG_VALUE)
					bPercent=true;
				else
				{
					poutStr[taridx++] = cur;
					poutStr[taridx] = 0;
				}
			}
		}

		return true;
	}

	const char* GetStr( const _T& code )
	{
		typename std::map<_T, std::string>::iterator it = m_StringMap.find( code );
		if( m_StringMap.end() == it ){
			return "nomsg";
		}
		
		return it->second.c_str();
	}
};