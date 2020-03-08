//
//	Common Open(Save) Dialog Module
//
//                                              written by lee jang ho
//
//////////////////////////////////////////////////////////////////////
#ifndef _OPENDLG_H
#define _OPENDLG_H

#include <windows.h>

// Modal OpenDlg
//		pFileName	:	이름을 저장할 스트링 버퍼
//						Allow Multiple Selection인 경우 ( bMultiSel=TRUE ) 디렉토리명뒤 \0 그리고 파일명 리스트가 \0으로 구분되어 온다.
//		nLen		:	pFileName 버퍼의 크기
//		pTitle		:	Caption Title
//		pFilter		:	Filter
//		pDefExt		:	Default Extension
//		bOpen		:	OpenDlg(TRUE) or SaveDlg(FALSE)
//		bMultiSel	:	Allo Multiple Selection(TRUE) at OpenDlg
BOOL OpenDlg(HWND hwnd, char *pFileName, int nLen, char *pTitle, char *pFilter, char *pDefExt, BOOL bOpen=TRUE, BOOL bMultiSel=FALSE);

// pFileName으로 얻어낸 패스가 Multi Selection으로 얻어낸경우
// GetDirFirst로 디렉토리를 얻고(NULL인 경우는 실패)
// GetNextFile로 NULL이 나올때까지 파일명을 얻어낸다.
/*
char *GetDirFirst(char *pFileNames);
char *GetNextFile(void);
int GetFileCount(char *pFileName);
*/

int GetFileNameCount(const char *pFileNames);
const char *GetFileName(const char *pFileNames, int idx);


#endif
