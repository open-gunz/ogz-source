/*
 *	CMError.h
 *		Error처리를 위한 함수 및 에러 코드 정의
 *		이장호 ( 98-01-04 1:19:44 오전 )
 *
 *		SetError(CodeNum)
 *		SetErrors(CodeNum,SubStr)
 *			를 사용해 에러 상태를 저장한다.
 ********************************************************************/

#ifndef _CMERROR_H
#define _CMERROR_H

#include "CMErrorDef.h"

#ifdef	_WIN32
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	_KOREAN_VER		/*	Korean Version	*/

	#ifdef __BORLANDC__

		#define SetError(_nErrCode)	\
			_SetError(_nErrCode,NULL,__FILE__,__LINE__,__TIME__,KSTR_##_nErrCode)
		#define SetErrors(_nErrCode,_pErrSubStr)	\
			_SetError(_nErrCode,_pErrSubStr,__FILE__,__LINE__,__TIME__,KSTR_##_nErrCode)

		#define ERROR_MESSAGE_TITLE		"에러"

	#else

		#define SetError(_nErrCode)	\
		_SetError(_nErrCode,NULL,__FILE__,__LINE__,__TIMESTAMP__,KSTR_##_nErrCode)
		#define SetErrors(_nErrCode,_pErrSubStr)	\
			_SetError(_nErrCode,_pErrSubStr,__FILE__,__LINE__,__TIMESTAMP__,KSTR_##_nErrCode)
	
		#define ERROR_MESSAGE_TITLE		"에러"

	#endif	// __BORLANDC__

#else	/*	English Version	(Default)	*/

	#ifdef __BORLANDC__

		#define SetError(_nErrCode)	\
			_SetError(_nErrCode,NULL,__FILE__,__LINE__,__TIME__,ESTR_##_nErrCode)
		#define SetErrors(_nErrCode,_pErrSubStr)	\
			_SetError(_nErrCode,_pErrSubStr,__FILE__,__LINE__,__TIME__,ESTR_##_nErrCode)
		#define ERROR_MESSAGE_TITLE		"Error"

	#else

		#define SetError(_nErrCode)	\
			_SetError(_nErrCode,NULL,__FILE__,__LINE__,__TIMESTAMP__,ESTR_##_nErrCode)
		#define SetErrors(_nErrCode,_pErrSubStr)	\
			_SetError(_nErrCode,_pErrSubStr,__FILE__,__LINE__,__TIMESTAMP__,ESTR_##_nErrCode)
		#define ERROR_MESSAGE_TITLE		"Error"

	#endif // __BORLANDC__

#endif

/*
에러 지정
	nErrCode			에러 코드
	pErrSubStr			에러 코드에 따른 부가 스트링
	pFileName			에러가 일어난 파일 명(__FILE__)
	nLineNum			에러가 일어난 라인 수(__LINE__)
	pLastModification	에러가 일어난 파일의 최근 수정일(__TIMESTAMP__)
	pErrStr				에러 스트링(에러코드 + _KSTR or _ESTR)
*/
void _SetError(int nErrCode,const char *pErrSubStr,const char *pFileName,int nLineNum,const char *pLastModification,const char *pErrStr);

/*
부가 에러 스트링 지정
	pErrSubStr			에러 코드에 따른 부가 스트링
*/
void SetErrorSubStr(const char *pErrSubStr);
/*
에러 코드 얻기
*/
int GetErrorCode(void);
/*
에러 스트링 얻기
*/
char *GetErrorString(void);
/*
부가 에러 스트링 얻기
*/
char *GetErrorSubString(void);
/*
에러가 난 파일명 얻기
*/
char *GetFileName(void);
/*
에러가 난 라인 수 얻기
*/
int GetLineNumber(void);
/*
에러가 난 파일의 최근 수정 일 얻기
*/
char *GetLastModification(void);

#ifdef	_WIN32
/*
에러 메세지 출력
*/
void ErrMsgBox(HWND hWnd);
#endif

#ifdef __cplusplus
}
#endif


#endif	/*	_CMERROR_H	*/
