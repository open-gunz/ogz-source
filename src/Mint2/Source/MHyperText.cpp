#include "stdafx.h"
/*
	MHyperText.cpp
	--------------
	
	MHyperText는 아주 단순한 형식을 가지고 있기 때문에 제대로 된 파서/스캐너의 형태를 가지고 있지는 않다.

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment Inc.
*/
#include "MWidget.h"
#include "MScrollBar.h"
#include "MHyperText.h"

#include <stdio.h>
#include <ctype.h>

#define TINVALID		-1		// 에러
#define TINTEGER		0		// 정수값
#define TCOLORVAL		1		// 컬러값, #뒤에 6개의 16진수가 나온다.
#define TSTRING			2		// 문자열 "" 안에 들어가있는 스트링

#define TTAGSTART		5		// <, 태그 시작 표시, 스캐닝이 행해진 후 이 토큰이 떨어지고 m_nOffset값이 0이 아니라면
								// 출력할 텍스트의 리스트가 존재함을 말하는 것이다.
#define TTAGEND			6		// >, 태그 끝 표시
#define TEQUAL			7		// EQUAL

// TT로 시작하는 것은 TAG를 위한 토큰 상수이다.
#define TTSTD			50		// STD TAG, 파일이 MAIET STD규약을 지키고 있음을 나타낸다.
#define TTIMAGE			51		// IMAGE TAG, 외부 이미지 파일과 링크되어 있음을 나타낸다.
#define TTSTYLE			52		// STYLE TAG, 이미지나 텍스트 파일의 스타일을 나타낸다.
#define TTLINK			53		// 링크를 위한 TAG
#define TTBR			54		// 다음줄로...
#define TTDEFAULT		55		// 초기 설정으로
#define TTLINKEND		56		// LINK의 끝을 알리는 태그

// TR로 시작하는 것은 TAG의 변수들을 위한 것이다. ARGUMENTs
#define TRBACKGROUND	100		// Background 변수 (지원 : STD)
#define TRCOLOR			101		// Color 변수 (지원 : STD, STYLE)
#define TRSIZE			102		// 크기 (지원 : STYLE)
#define TRALIGN			103		// 정렬 (지원 : STYLE)
#define TRTYPE			104		// 링크의 종류
#define TRSRC			105		// 소스 데이터 위치
#define TRBOLD			106		// 폰트를 위한 것. (BOLD?)
#define TRHIGHLIGHT		107		// 하이라이트 컬러 (지원 : STD, STYLE)
#define TRHREF			108
#define TRXMARGIN		109
#define TRYMARGIN		110
//#define TRFACE			106		// 폰트 이름, 아직 지원하지 않음.

// 여기부터선 스타일 정의를 위한 상수값들
#define TRLEFT			150
#define TRCENTER		151
#define TRRIGHT			152

#define TROPEN			153		// 다른 STD파일을 연다.
#define TRWWW			154		// 웹브라우즈를 위한 것
#define TREXEC			155		// 실행을 위한 것
#define TRYES			156
#define TRNO			157
#define TRTRUE			158
#define TRFALSE			159

#define TENDDOC			254		// EOS(END-OF-STRING) 또는 EOF(END-OF-FILE) 값일때 이것으로 종료된다.
								// 이 토큰 역시 m_nOffset값이 0이 아닐때 출력할 텍스트가 존재한다.

#define MAKERGB(r,g,b)			((DWORD)(((BYTE)(b)|((WORD) (g) << 8))|(((DWORD)(BYTE)(r)) << 16)))

static struct _reserved_word {
	int			nToken;
	char*		szText;
} Reserved[] = {
		/* TAG */
	{ TTSTD,		"std"		},
	{ TTIMAGE,		"image"		},
	{ TTSTYLE,		"style"		},
	{ TTLINK,		"link"		},
	{ TTLINKEND,	"linkend"	},
	{ TTBR,			"br"		},
	{ TTDEFAULT,	"default"	},

		/* TAG variable */
	{ TRBACKGROUND,	"background"},			// BACKGROUND, STD의 style
	{ TRCOLOR,		"color"		},			//
	{ TRSIZE,		"size"		},			//
	{ TRALIGN,		"align"		},			//
	{ TRTYPE,		"type"		},			//
	{ TRSRC,		"src"		},			// Source Data
	{ TRBOLD,		"bold"		},			//
	{ TRHIGHLIGHT,	"highlight"	},			//
	{ TRHREF,		"href"		},			// Reference
	{ TRXMARGIN,	"xmargin"	},			//
	{ TRYMARGIN,	"ymargin"	},			//

		/* Misc. */
	{ TINVALID,		NULL,		},
};

static struct _reserved_word_constant {
	int			nToken;
	char*		szText;
} Constant[] = {
		/* TAG variable constants */
	{ TRLEFT,		"left"		},			// left align.
	{ TRCENTER,		"center"	},			// center align.
	{ TRRIGHT,		"right"		},			// right align.

	{ TROPEN,		"open"		},			// open another STD file
	{ TRWWW,		"www"		},			// web browsing option
	{ TREXEC,		"exec"		},			// execution

	{ TRYES,		"yes"		},
	{ TRNO,			"no"		},

	{ TRTRUE,		"true"		},
	{ TRFALSE,		"false"		},

		/* Misc. */
	{ TINVALID,		NULL		},
};

static u32 dwScanColor = 0;

static char *fold( char *s )
{
	int i;
	for(i=0; s[i]!='\0'; i++)
		if(isupper(s[i])) s[i] = s[i] + ('a'-'A');
    
	return(s);
}


//#define _HT_LOG_ENABLE_

#ifdef _HT_LOG_ENABLE_
//TODO:Delete...
#include "MDebug.h"
#endif


int	MHyperText::Scan()
{
	int ret = TINVALID, i;	//
	char c;					//Temporary Data
	bool bFirstSpace = false;

	m_nOffset = 0;

	if(m_bTagReady == false)
	{
read_plain_text_loop:
		c = Input();

		//일반 텍스트로 간주하고 무조건 읽어 들인다.
		if((c=='\t'||c==' '||c=='\n') && bFirstSpace == true)
		{
			//공란 합치기
			for(c = Input(); (c=='\t'||c==' '||c=='\n'); );
			Unput();
			Gather(' ');
			goto read_plain_text_loop;
		}
		else if(c=='<')
		{
			//태그가 시작되었다.
			m_bTagReady = true;		//태그 스타트
			Gather('\0');			//텍스트버퍼 종료
			return TTAGSTART;
		}
		else if(c==EOF || c=='\0')	//만일 m_bTagReady가 true인 상태에서 이값을 받았다면 그것은 에러다.
		{	//끝
			Gather('\0');
			return TENDDOC;
		}
		else
		{
			if(bFirstSpace == false){
				bFirstSpace = true;
			}
			Gather(c);
			goto read_plain_text_loop;
		}
	}
	else 
	{
		//태그가 시작되었으므로 태그와 관련된 스캐닝을 한다.
		//태그에 관련된 것이므로 알아 먹지 못하는 값이 들어오면 무조건 에러다.
read_tag_loop:
		c = Input();

		if(c=='\t' || c==' ' || c=='\n'){		//공란 무시하기
			goto read_tag_loop;
		}
		else if(c=='>'){
			m_nOffset = 0; Gather('\0');
			m_bTagReady = false;
			return TTAGEND;
		}
		else if(isdigit(c)){					//숫자가 들어왔다.
			m_nOffset = 0;
			GetDigit(c);
			Gather('\0');
			return TINTEGER;
		}
		else if(c==EOF || c=='\0'){				//태그도 끝나지 않았는데 이런 일이 생긴다면 그것 역시 에러다.
			m_nOffset = 0;
			Gather('\0');
			return TINVALID;
		}
		else if(c=='#'){						//컬러값이 들어왔다.
			u8 nHigh, nLow;
			u8 rgb[3];

			for(i=0;i<3;i++){				
				c = Input();

				if(isdigit(c) == 0){
					if((c < 'a' || c > 'z') && (c < 'A' || c > 'Z')){
						Gather('\0');
						return TINVALID;
					}else{
						if(c >= 'a' && c <= 'z') nHigh = 10 + (c - 'a'); else nHigh = 10 + (c - 'A');
					}
				}else{
					nHigh = c - 48;	/* 48은 0의 아스키코드 */
				}

				c = Input();

				if(isdigit(c) == 0){
					if((c < 'a' || c > 'z') && (c < 'A' || c > 'Z')){
						Gather('\0');
						return TINVALID;
					}else{
						if(islower(c)) nLow = 10 + (c - 'a'); else nLow = 10 + (c - 'A');
					}
				}else{
					nLow = c - 48;
				}
				rgb[i] = nHigh*16 + nLow;
			}
			dwScanColor = MINT_RGB(rgb[0],rgb[1],rgb[2]);
			
			c = Input();
			if(isdigit(c) == 0){
				Unput();
				Gather('\0');
#ifdef _HT_LOG_ENABLE_
				rslog("COLORVAL : %d (%d,%d,%d)\n",dwScanColor, rgb[0], rgb[1], rgb[2]);
#endif
				return TCOLORVAL;
			} else {
				return false;
			}
		}	// if( # )
		else if(isalpha(c))		// CHECK RESERVED WORD
		{
			m_nOffset = 0;
			Gather(c);
			for(c=Input(); isalpha(c) || isdigit(c); c=Input()) Gather(c);
			Unput();
			Gather('\0');

			fold(m_szScan);
			for(i=0; Reserved[i].szText != NULL; i++){
				if(strcmp(m_szScan, Reserved[i].szText) == 0){
					break;
				}
			}

			if(Reserved[i].szText != NULL)
			{
				return Reserved[i].nToken;
			}
			else
			{	//상수 체크
				for(i=0; Constant[i].szText != NULL; i++){
					if(strcmp(m_szScan, Constant[i].szText) == 0){
						break;
					}
				}
				if(Constant[i].szText != NULL) {
					return Constant[i].nToken;
				} else {
					//Undefined Symbol : ERROR!
					m_nOffset = 0;
					Gather('\0');
					return TINVALID;
				}
			}
		}
		else if(c == '\"')
		{	// 이건 문자열이다.
			m_nOffset = 0;
			for(c = Input(); ; c = Input()){
				if(c == '\"'){
					if((c=Input()) == '\"'){
						Gather('\\');
						Gather('\"');
					} else {
						break;
					}
				} else {
					Gather(c);
				}
			}
			Unput();
			Gather('\0');
			return TSTRING;
		}
		else if(c == '=')
		{
			return TEQUAL;
		}
	}
	return ret;
}

MHyperText::MHyperText()
{
	m_pBuffer = NULL;
	m_szScan[0] = '\0';
}

bool MHyperText::Parse()
{
	//태그와 Plain-Text를 읽어내는 부분을 STD에서는 Element라고 부른다.
	MHyperTextElement*	pElement = NULL;
	MHyperTextArg*		pArg = NULL;
	//현재 파싱하는 데이터를 위한 토큰
	int nToken, nVal;

	//초! 허접 파싱을 위하여 쓰이는 2개의 변수들
	int nLastArg = TINVALID;
	bool bEqual = false;

	m_bTagReady	= false;			//스캔하기 전에 일단 모든 값을 리셋
	bp			= 0;
	m_nOffset	= 0;

	do {
		m_nOffset = 0;

		if((nToken = Scan()) == TINVALID) return false;

		if(nToken == TTAGSTART || nToken == TENDDOC){

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MAKE PLAIN TEXT LIST

			if(strcmp(m_szScan, "") != 0 && strcmp(m_szScan, " ") != 0 && strcmp(m_szScan, "\n") != 0){
				pElement = new MHyperTextElement(MHTE_PLAINTEXT);
				pArg = new MHTA_Text(m_szScan);

				if(pArg){
					pElement->Add(pArg);
					Elements.Add(pElement);
				}
				pElement = NULL;
				pArg = NULL;
			}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		} else {

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MAKE TAG LIST

			switch(nToken)
			{
			case TTAGEND :		//태그가 끝났다.
				if(pElement != NULL) Elements.Add(pElement);
				pElement	= NULL;
				pArg		= NULL;
				break;


			case TTSTD :		//TAG
				if(pElement != NULL) goto parse_failure;	//태그가 존재한다.
				pElement = new MHyperTextElement(MHTE_STD);
				break;
			case TTIMAGE :		//TAG
				if(pElement != NULL) goto parse_failure;	//태그가 존재한다.
				pElement = new MHyperTextElement(MHTE_IMAGE);
				break;
			case TTSTYLE :		//TAG
				if(pElement != NULL) goto parse_failure;	//태그가 존재한다.
				pElement = new MHyperTextElement(MHTE_STYLE);
				break;
			case TTLINK :		//TAG
				if(pElement != NULL) goto parse_failure;	//태그가 존재한다.
				pElement = new MHyperTextElement(MHTE_LINK);
				break;
			case TTLINKEND :
				if(pElement != NULL) goto parse_failure;
				pElement = new MHyperTextElement(MHTE_LINKEND);
				break;
			case TTBR :			//TAG
				if(pElement != NULL) goto parse_failure;	//태그가 존재한다.
				pElement = new MHyperTextElement(MHTE_BR);
				break;
			case TTDEFAULT :
				if(pElement != NULL) goto parse_failure;
				pElement = new MHyperTextElement(MHTE_DEFAULT);
				break;


			case TRBACKGROUND :	//ARG
			case TRCOLOR :		//ARG
			case TRSIZE :		//ARG
			case TRALIGN :		//ARG
			case TRTYPE :		//ARG
			case TRSRC :		//ARG
			case TRBOLD :		//ARG
			case TRHIGHLIGHT :	//ARG
			case TRHREF :		//ARG
			case TRXMARGIN :	//ARG
			case TRYMARGIN :	//ARG
				//태그가 지정되었는가와 '='의 뒤에 나오지 않도록 체크한다.
				if(pElement == NULL || bEqual == true) goto parse_failure;
				nLastArg = nToken;
				break;


			case TEQUAL :		//EQUAL은 L-VALUE가 요구된다.
				if(bEqual==true || pElement==NULL) goto parse_failure;
				bEqual = true;
				break;


			case TINTEGER :		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				
				nVal = atoi(m_szScan);

				switch(nLastArg){
				case TRSIZE:	pArg = new MHTA_Size(nVal);		break;
				case TRALIGN:	pArg = new MHTA_Align(nVal);	break;
				case TRTYPE:	pArg = new MHTA_Type(nVal);		break;
				case TRBOLD:	pArg = new MHTA_Bold(nVal);		break;
				case TRXMARGIN:	pArg = new MHTA_XMargin(nVal);	break;
				case TRYMARGIN:	pArg = new MHTA_YMargin(nVal);	break;
				default:		goto parse_failure;
				}

				if(pArg != NULL) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TCOLORVAL:		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;

				switch(nLastArg){
				case TRBACKGROUND:
					pArg = new MHTA_Background(dwScanColor); break;
				case TRCOLOR:
					pArg = new MHTA_Color(dwScanColor); break;
				case TRHIGHLIGHT:
					pArg = new MHTA_Highlight(dwScanColor); break;
				default:
					goto parse_failure;
				}

				if(pArg != NULL) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TSTRING :		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				
				switch(nLastArg){
				case TRBACKGROUND:	pArg = new MHTA_Background(m_szScan); break;
				case TRSRC :		pArg = new MHTA_Src(m_szScan); break;
				case TRHREF :		pArg = new MHTA_HRef(m_szScan); break;
				default :			goto parse_failure;
				}

				if(pArg != NULL) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TRYES :
			case TRNO :
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRBOLD) goto parse_failure;

				if(nToken == TRYES){
					nVal = 1;
				} else if(nToken == TRNO){
					nVal = 0;
				}

				pArg = new MHTA_Bold(nVal);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TRTRUE :
			case TRFALSE :
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRBOLD) goto parse_failure;

				if(nToken == TRTRUE){
					nVal = 1;
				} else if(nToken == TRFALSE){
					nVal = 0;
				}

				pArg = new MHTA_Bold(nVal);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TRLEFT :		//R-VALUE
			case TRCENTER :		//R-VALUE
			case TRRIGHT :		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRALIGN) goto parse_failure;

				nVal = 0;
				
				if(nToken == TRLEFT){
					nVal = 0;
				}else if(nToken == TRCENTER){
					nVal = 1;
				}else if(nToken == TRRIGHT){
					nVal = 2;
				}

				pArg = new MHTA_Align(nVal);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TROPEN :
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRTYPE) goto parse_failure;
				pArg = new MHTA_Type(0);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TRWWW :		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRTYPE) goto parse_failure;
				pArg = new MHTA_Type(1);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;

			case TREXEC :		//R-VALUE
				if(pElement == NULL || bEqual==false || nLastArg == TINVALID) goto parse_failure;
				if(nLastArg != TRTYPE) goto parse_failure;
				pArg = new MHTA_Type(2);
				if(pArg) pElement->Add(pArg);
				pArg = NULL;
				bEqual = false;
				break;
			default : goto parse_failure;
			}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		}
	} while(nToken != TENDDOC);

	return true;

parse_failure:
	if(bEqual == true && pArg != NULL) delete pArg;
	if(pElement) delete pElement;

	return false;
}

bool MHyperText::Open(char *szTextBuffer)
{
	bool ret;

	if(szTextBuffer == NULL){
		Close();
		return false;
	}

	Close();

	m_nLen = strlen(szTextBuffer);
	m_pBuffer = _strdup(szTextBuffer);

	ret = Parse();
	if(ret == false){
		Close();
		return false;
	}

#ifdef _HT_LOG_ENABLE_
	for(int i=0; i<Elements.GetCount(); i++)
	{
		MHyperTextElement *pEl = Elements.Get(i);
		rslog("[%d] TYPE:%d [Argument(count = %d) ", i, pEl->nType, pEl->Args.GetCount());

		for(int j=0; j<pEl->Args.GetCount(); j++)
		{
			MHyperTextArg* pArg = pEl->Args.Get(j);
			if(pArg){
				rslog(" [%d]", pArg->uId);
			}
		}
		rslog("]\n");
	}
#endif

	//파싱후 STD가 선두에 위치할 수 있도록 리스트를 정리한다.
	while(Elements.GetCount() > 0){
		MHyperTextElement *pEl = Elements.Get(0);
		if(pEl->nType != MHTE_STD) Elements.Delete(0); else break;
	}
	if(Elements.GetCount() == 0){
		Close();
		return false;
	}
	
	return ret;
}

void MHyperText::Close()
{
	m_nLen = 0;
	bp = 0;

	Elements.DeleteAll();

	if(m_pBuffer){ free(m_pBuffer); m_pBuffer = NULL; }
}

bool MHyperTextFile::Open(char *szPath)
{
	FILE *fp;
	long lFileSize;
	int i, ch;

	fopen_s(&fp, szPath, "rt");

	if(fp){
		fseek(fp, 0, SEEK_END);
		lFileSize = ftell(fp);

		if(lFileSize == 0){
			fclose(fp);
			return false;
		}

		fseek(fp, 0, SEEK_SET);

		m_pBuffer = new char[lFileSize+1];	
		i = 0;

		do {
			ch = fgetc(fp);
			if(ch != EOF) m_pBuffer[i] = (char)ch; else m_pBuffer[i] = '\0';
			i++;
		} while(feof(fp) == 0);
		fclose(fp);
		
		return true;
	}
	return false;
}








