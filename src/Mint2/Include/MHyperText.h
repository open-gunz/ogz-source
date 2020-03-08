/*
	MHyperText.h
	Programming by Joongpil Cho

	간이 파일 포맷을 사용하고 있음.	일반적인 텍스트와 유사하나 [[...]]안의 명령어를 통해서 텍스트의 형식을 
	나타내거나 그림을 삽입할 수 있다.
*/
#include <crtdbg.h>
#include <stdio.h>
#include "CMList.h"

#ifndef __HYPERTEXT_HEADER__
#define __HYPERTEXT_HEADER__

typedef enum {
	MHTE_PLAINTEXT = 0,	//TAG없는 일반 텍스트. (태그 아님)
	MHTE_STD,			//MAIET Hyper Text임을 표시하는 태그
	MHTE_IMAGE,			//이미지 태그
	MHTE_STYLE,			//스타일 태그
	MHTE_LINK,			//하이퍼 링크 시작
	MHTE_LINKEND,		//하이퍼 링크의 끝
	MHTE_BR,			//다음 줄로...
	MHTE_DEFAULT,		//초기설정대로
} MHT_ELEMENT;

typedef enum {
	MHTA_TEXT = 0,		//PLAINTEXT에 대한 인자, char*를 담고 있다.
	MHTA_BACKGROUND,	//STD에 대한 인자 컬러 값 혹은 이미지 파일이 올수 있다.
	MHTA_COLOR,			//COLOR값, #으로 시작하는 16진수 6자리, 각 2자리가 하나의 색상정보를 표현한다. (#RGB)
	MHTA_SIZE,			//SIZE값, 정수형()
	MHTA_ALIGN,			//ALIGN값, 정수형()
	MHTA_TYPE,			//TYPE값, 정수형()
	MHTA_SRC,			//SRC값, 문자열
	MHTA_BOLD,			//BOLD값, 정수형()
	MHTA_HIGHLIGHT,
	MHTA_HREF,			//링크 리퍼런스, 문자열
	MHTA_XMARGIN,		//그림의 X축 여분
	MHTA_YMARGIN,		//그림의 Y축 여분
} MHT_ARGUMENT;

// 각각의 Text Element에 대한 보조수치들의 값
class MHyperTextArg {
public:
	MHT_ARGUMENT		uId;			// 엘리먼트 아규먼트
	
	MHyperTextArg(MHT_ARGUMENT id){
		uId = id;
	}
};

template<MHT_ARGUMENT Arg>
class MHTA_IntegerArg : public MHyperTextArg {
public:
	int					nVal;

	MHTA_IntegerArg(int value) : MHyperTextArg(Arg){
		nVal = value;
	}
};

template<MHT_ARGUMENT Arg>
class MHTA_StringArg : public MHyperTextArg {
public:
	char*				val;

	MHTA_StringArg(char *szText):MHyperTextArg(Arg){
		val = _strdup(szText);
	}
	~MHTA_StringArg(){
		if(val) free(val);
	}
};

template<MHT_ARGUMENT Arg>
class MHTA_ColorArg : public MHyperTextArg {
public:
	MCOLOR				sColor;

	MHTA_ColorArg(MCOLOR color) : MHyperTextArg(Arg){
		sColor = color;
	}
};

class MHTA_Background : public MHyperTextArg {
	MHTA_Background() : MHyperTextArg(MHTA_BACKGROUND){
		sColor = MCOLOR(0,0,0);
		szPath = NULL;
	}
public:
	char*				szPath;		//이미지 패스
	MCOLOR				sColor;

	MHTA_Background(MCOLOR color) : MHyperTextArg(MHTA_BACKGROUND){
		szPath = NULL;
		sColor = color;
	}

	MHTA_Background(char *path) : MHyperTextArg(MHTA_BACKGROUND){
		sColor = MCOLOR(0,0,0);
		szPath = _strdup(path);
	}

	~MHTA_Background(){
		if(szPath) free(szPath);	
	}
};

typedef class MHTA_IntegerArg<MHTA_SIZE>	MHTA_Size;
typedef class MHTA_IntegerArg<MHTA_ALIGN>	MHTA_Align;
typedef class MHTA_IntegerArg<MHTA_TYPE>	MHTA_Type;
typedef class MHTA_IntegerArg<MHTA_BOLD>	MHTA_Bold;
typedef class MHTA_IntegerArg<MHTA_XMARGIN>	MHTA_XMargin;
typedef class MHTA_IntegerArg<MHTA_YMARGIN>	MHTA_YMargin;

typedef class MHTA_ColorArg<MHTA_COLOR>		MHTA_Color;
typedef class MHTA_ColorArg<MHTA_HIGHLIGHT>	MHTA_Highlight;

typedef class MHTA_StringArg<MHTA_TEXT>		MHTA_Text;
typedef class MHTA_StringArg<MHTA_SRC>		MHTA_Src;
typedef class MHTA_StringArg<MHTA_HREF>		MHTA_HRef;


class MHyperTextElement
{
public:
	MHT_ELEMENT					nType;	// 엘리먼트의 타입
	CMLinkedList<MHyperTextArg>	Args;	// 엘리먼트의 인자 리스트

	//생성자, 파괴자
	MHyperTextElement(MHT_ELEMENT type){
		nType		= type;
	}

	virtual ~MHyperTextElement(){
		Args.DeleteAll();
	}

	void Add(MHyperTextArg* pNew){
		Args.Add(pNew);
	}
};

class MHyperText
{
private:
	char*			m_pBuffer;			// Text Buffer, MHyperText는 메모리에 있는 내용만을 파싱한다.
	int				m_nLen;				// 버퍼의 크기
	int				bp;					// Buffer의 포인터
	int				m_nOffset;			// m_szScan의 버퍼포인터
	bool			m_bTagReady;
	char			m_szScan[20480];	// Scan한 값이 저장되는 문자열 포인터, yytext와 유사한 역할을 하는 놈이다.

	void			Gather(char b){ m_szScan[m_nOffset++] = b; }
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS

	int				Scan();
	bool			Parse();
	char			Input(){ return m_pBuffer[bp++]; }
	void			Unput(){ bp--; }
	void			GetDigit(char c);

public:
	CMLinkedList<MHyperTextElement>	Elements;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	
	MHyperText();
	virtual ~MHyperText(){ Close(); }

	/*	
		이 클래스는 에러 리포트를 하지 않는다.
		하이퍼 텍스트가 게임내에 적용되었을 때에도 게임은 계속되어야 하므로...

		szTextBuffer : 텍스트 버퍼
	*/
	bool Open(char *szTextBuffer);
	void Close();

	bool IsValid(){ return (m_pBuffer==NULL)?false:true; }
};

inline void MHyperText::GetDigit(char c)
{
	for(;isdigit(c) ;c=Input()) Gather(c);
	Unput();
}

class MHyperTextFile
{
private:
	char*	m_pBuffer;
public:
	MHyperTextFile()
	{
		m_pBuffer = NULL;
	}

	virtual ~MHyperTextFile()
	{
		Close();
	}

	bool Open(char *szPath);

	char* GetBuffer(){ return m_pBuffer; }

	void Close()
	{
		if(m_pBuffer == NULL)
		{
			delete m_pBuffer;
			m_pBuffer = NULL;
		}
	}
};

#endif